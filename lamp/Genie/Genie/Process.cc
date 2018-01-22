/*	==========
 *	Process.cc
 *	==========
 */

#include "Genie/Process.hh"

// Mac OS X
#ifdef __APPLE__
#include <CoreServices/CoreServices.h>
#endif

// Mac OS
#ifndef __MACTYPES__
#include <MacTypes.h>
#endif

// Standard C++
#include <vector>

// Standard C
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>

// POSIX
#include <fcntl.h>
#include "sys/stat.h"
#include "sys/wait.h"
#include "unistd.h"

// mac-sys-utils
#include "mac_sys/exit_to_shell.hh"

// relix-include
#include "relix/syscalls.h"

// Iota
#include "iota/strings.hh"

// gear
#include "gear/parse_decimal.hh"

// Debug
#include "debug/assert.hh"

// plus
#include "plus/var_string.hh"

// Recall
#include "recall/stack_frame.hh"

// Nitrogen
#include "Mac/Sound/Functions/SysBeep.hh"

// Recall
#include "recall/backtrace.hh"

// poseven
#include "poseven/types/errno_t.hh"

// vfs
#include "vfs/file_descriptor.hh"
#include "vfs/filehandle.hh"
#include "vfs/node.hh"
#include "vfs/filehandle/primitives/pread.hh"
#include "vfs/functions/resolve_links_in_place.hh"
#include "vfs/functions/resolve_pathname.hh"
#include "vfs/primitives/open.hh"
#include "vfs/primitives/stat.hh"

// MacVFS
#include "MacVFS/util/get_Mac_type_code.hh"

// relix-kernel
#include "relix/api/deliver_fatal_signal.hh"
#include "relix/api/getcwd.hh"
#include "relix/api/get_process_group.hh"
#include "relix/api/root.hh"
#include "relix/api/terminate_current_process.hh"
#include "relix/api/waits_for_children.hh"
#include "relix/config/mini.hh"
#include "relix/config/syscall_stacks.hh"
#include "relix/fs/console.hh"
#include "relix/glue/system_call.68k.hh"
#include "relix/glue/system_call.ppc.hh"
#include "relix/glue/userland.hh"
#include "relix/signal/signal.hh"
#include "relix/signal/signal_process_group.hh"
#include "relix/signal/signal_traits.hh"
#include "relix/task/A5_world.hh"
#include "relix/task/alarm_clock.hh"
#include "relix/task/fd_map.hh"
#include "relix/task/process.hh"
#include "relix/task/process_group.hh"
#include "relix/task/process_image.hh"
#include "relix/task/process_resources.hh"
#include "relix/task/scheduler.hh"
#include "relix/task/session.hh"
#include "relix/time/cpu_time_checkpoint.hh"

// Genie
#include "Genie/Faults.hh"
#include "Genie/ProcessList.hh"
#include "Genie/Process/AsyncYield.hh"


#ifndef SIGSTKFLT
#define SIGSTKFLT  (-1)
#endif


static void DumpBacktrace()
{
	using namespace recall;
	
	const unsigned frame_capacity = 64;
	
	frame_data stack_crawl[ frame_capacity ];
	
	const unsigned n_frames = make_stack_crawl( stack_crawl, frame_capacity );
	
	const frame_data* begin = stack_crawl;
	const frame_data* end   = stack_crawl + n_frames;
	
	++begin;  // skip DumpBacktrace( void )
	
	plus::var_string report;
	
	make_report_from_stack_crawl( report, begin, end );
	
	(void) relix::console::log( report.data(), report.size() );
}

namespace relix
{
	
	Genie::Process* gCurrentProcess;  // extern, declared in Faults.cc
	
}

namespace Genie
{
	
	namespace p7 = poseven;
	
	
	using relix::memory_data;
	
	
	static uint64_t microseconds()
	{
		return clock();
	}
	
	extern "C" _relix_system_parameter_block global_parameter_block;
	
	_relix_system_parameter_block global_parameter_block =
	{
		NULL,  // current user
		
		sizeof (_relix_system_parameter_block),
		sizeof (_relix_user_parameter_block),
		
		TARGET_CPU_68K ? &relix::dispatch_68k_system_call :
		TARGET_CPU_PPC ? &relix::dispatch_ppc_system_call
		               : NULL,
		
		&microseconds
	};
	
	using relix::gCurrentProcess;
	
	Process& CurrentProcess()
	{
		if ( gCurrentProcess == NULL )
		{
			p7::throw_errno( ESRCH );
		}
		
		return *gCurrentProcess;
	}
	
	
	struct signal_param
	{
		pid_t  pgid;
		int    signo;
	};
	
	static void* signal_process_in_group( void* param, pid_t, Process& process )
	{
		signal_param& pb = *(signal_param*) param;
		
		if ( process.GetPGID() == pb.pgid )
		{
			process.Raise( pb.signo );
		}
		
		return NULL;
	}
	
	void SendSignalToProcessGroup( int sig, pid_t pgid )
	{
		signal_param param = { pgid, sig };
		
		for_each_process( &signal_process_in_group, &param );
	}
	
	Process& GetProcess( pid_t pid )
	{
		return get_process( pid );
	}
	
	Process* FindProcess( pid_t pid )
	{
		return lookup_process( pid );
	}
	
}

namespace relix
{
	
	void deliver_fatal_signal( int signo )
	{
		using namespace Genie;
		
		typedef void (*signal_handler_t)(int);
		
		if ( gCurrentProcess != NULL )
		{
			signal_handler_t handler = gCurrentProcess->GetSignalAction( signo ).sa_handler;
			
			if ( handler != SIG_DFL  &&  handler != SIG_IGN )
			{
				relix::call_signal_handler( handler, signo );
			}
			
			gCurrentProcess->Terminate( signo | 0x80 );
		}
		
		if ( CONFIG_DEBUGGING )
		{
			::DebugStr( "\p" "Fatal condition occurred on main thread" );
		}
		else
		{
			Mac::SysBeep();
			Mac::SysBeep();
			Mac::SysBeep();
		}
		
		mac::sys::exit_to_shell();  // not messing around
	}
	
}

namespace Genie
{
	
	// This function doesn't return if the process is current.
	static inline
	void exit_process( Process& process, int exit_status )
	{
		const int wait_status = (exit_status & 0xFF) << 8;
		
		process.Terminate( wait_status );
	}
	
	static int reexec_start( void* args[] )
	{
		Reexec_Function f = (Reexec_Function) args[ 0 ];
		
		int exit_status = f( args[ 1 ],
		                     args[ 2 ],
		                     args[ 3 ],
		                     args[ 4 ],
		                     args[ 5 ],
		                     args[ 6 ],
		                     args[ 7 ] );
		
		return exit_status;
	}
	
	
	void* Process::thread_start( void* param, const void* bottom, const void* limit )
	{
		Process* process = reinterpret_cast< Process* >( param );
		
		relix::process_image& image = process->get_process().get_process_image();
		
		_relix_user_parameter_block& pb = image.initialize_param_block( bottom, limit );
		
		global_parameter_block.current_user = &pb;
		
		process->Resume();
		
		// Accumulate any system time between start and entry to main()
		relix::leave_system();
		
		int exit_status = 0;
		
		if ( process->itsReexecArgs[ 0 ] )
		{
			exit_status = reexec_start( process->itsReexecArgs );
		}
		else
		{
			exit_status = image.enter_start_routine( &global_parameter_block );
			
			// Not reached by regular tools, since they call exit()
		}
		
		// Accumulate any time between last syscall (if any) and return from userspace
		relix::enter_system();
		
		exit_process( *process, exit_status );
		
		// Not reached
		
		return NULL;
	}
	
	
	static std::size_t argv_length( char const* const* argv )
	{
		std::size_t result = 0;
		
		while ( *argv++ )
		{
			++result;
		}
		
		return result;
	}
	
	struct ExecContext
	{
		vfs::node_ptr               executable;
		std::vector< const char* >  argVector;
		plus::var_string            interpreter;
		
		ExecContext()  {}
		
		ExecContext( const vfs::node&    executable,
		             char const* const*  argv )
		:
			executable( &executable ),
			argVector ( argv, argv + argv_length( argv ) + 1 )
		{}
	};
	
	static inline p7::errno_t NotExecutable()  { return p7::errno_t( EPERM ); }
	
	static plus::string first_disk_block( const vfs::node& file )
	{
		const size_t buffer_length = 512;
		
		plus::var_string result;
		
		char* p = result.reset( buffer_length );
		
		ssize_t n_read = 0;
		
		try
		{
			const vfs::filehandle_ptr fh = open( file, O_RDONLY, 0 );
			
			n_read = pread( *fh, p, buffer_length, 0 );
		}
		catch ( ... )
		{
		}
		
		result.resize( n_read );
		
		return result.move();
	}
	
	static void Normalize( const char* path, ExecContext& context, const vfs::node& cwd )
	{
		OSType type = 0;
		
		try
		{
			type = vfs::get_Mac_type_code( *context.executable );
		}
		catch ( ... )
		{
			// Assume that non-FSSpec executables are binaries, not scripts
		}
		
		if ( type == 'Tool' )
		{
			return;  // Already normalized
		}
		
		const plus::string block = first_disk_block( *context.executable );
		
		const ssize_t bytes = block.size();
		
		const char* data = block.c_str();
		
		const bool has_shebang = bytes > 2 && data[0] == '#' && data[1] == '!';
		
		if ( type == 'TEXT'  ||  (type == 0  &&  has_shebang) )
		{
			if ( has_shebang )
			{
				const char* end = data + bytes;
				
				const char* cr = std::find( data, end, '\r' );
				const char* lf = std::find( data, end, '\n' );
				
				const char* nl = std::min( cr, lf );
				
				if ( nl == end )
				{
					throw NotExecutable();  // #! line too long
				}
				
				context.interpreter.assign( &data[ 2 ], nl );
			}
			else
			{
				context.interpreter = "/bin/sh";  // default
			}
			
			// E.g. "$ script foo bar baz"
			// argv == { "script", "foo", "bar", "baz", NULL }
			
			if ( std::strchr( context.argVector[ 0 ], '/' ) == NULL )
			{
				// Overwrite with path
				context.argVector[ 0 ] = path;
				
				// argv == { "/path/to/script", "foo", "bar", "baz", NULL }
			}
			
			char* data = &context.interpreter[ 0 ];
			
			context.argVector.insert( context.argVector.begin(), data );
			
			// argv == { "sh", "script", "foo", "bar", "baz", NULL }
			
			char* p = data;
			
			int i = 0;
			
			while ( char* space = strchr( p, ' ' ) )
			{
				*space = '\0';
				
				p = space + 1;
				
				context.argVector.insert( context.argVector.begin() + ++i, p );
			}
			
			plus::string path = context.interpreter.substr( 0, strlen( data ) );
			
			context.executable = resolve_pathname( *relix::root(), path, cwd );
		}
		else if ( type != 0 )
		{
			throw NotExecutable();
		}
	}
	
	
	static void* find_process_group( void* param, pid_t, Process& process )
	{
		const pid_t pgid = *(pid_t*) param;
		
		relix::process_group& process_group = process.get_process().get_process_group();
		
		if ( process_group.id() == pgid )
		{
			return &process_group;
		}
		
		return NULL;
	}
	
	static boost::intrusive_ptr< relix::process_image >
	//
	new_process_image( const vfs::node&    exe,
	                   const char* const*  argv,
	                   const char* const*  envp )
	{
		return new relix::process_image( exe, argv, envp );
	}
	
	static vfs::filehandle_ptr open_device( const char* path, size_t length )
	{
		return open( *vfs::resolve_absolute_path( *relix::root(), path, length ), O_RDWR, 0 );
	}
	
	Process::Process( RootProcess ) 
	:
		relix::thread( 1,
		               0,
		               *new relix::process(),
		               false ),
		itsPID                ( 1 ),
		itsForkedChildPID     ( 0 ),
		itsLifeStage          ( kProcessLive ),
		itsSchedule           ( kProcessSleeping ),
		itsResult             ( 0 ),
		itsAsyncOpCount       ( 0 ),
		itMayDumpCore         ()
	{
		itsReexecArgs[0] =
		itsReexecArgs[1] =
		itsReexecArgs[2] =
		itsReexecArgs[3] =
		itsReexecArgs[4] =
		itsReexecArgs[5] =
		itsReexecArgs[6] =
		itsReexecArgs[7] = NULL;
		
		relix::fd_map& fds = FileDescriptors();
		
		fds[ 0 ] =
		fds[ 1 ] = open_device( STR_LEN( "/dev/null"    ) );
		fds[ 2 ] = open_device( STR_LEN( "/dev/console" ) );
		
		relix::InstallExceptionHandlers();
		
		relix::save_the_A5_world();
	}
	
	Process::Process( Process& parent, pid_t pid, pid_t tid ) 
	:
		relix::thread( tid,
		               parent.signals_blocked(),
		               tid == pid ? *new relix::process( pid, parent.get_process() )
		                          : parent.get_process(),
		               false ),
		itsPID                ( pid ),
		itsForkedChildPID     ( 0 ),
		itsLifeStage          ( kProcessStarting ),
		itsSchedule           ( kProcessRunning ),
		itsResult             ( 0 ),
		itsAsyncOpCount       ( 0 ),
		itMayDumpCore         ( true )
	{
		itsReexecArgs[0] =
		itsReexecArgs[1] =
		itsReexecArgs[2] =
		itsReexecArgs[3] =
		itsReexecArgs[4] =
		itsReexecArgs[5] =
		itsReexecArgs[6] =
		itsReexecArgs[7] = NULL;
		
		relix::mark_thread_active( tid );
	}
	
	Process& Process::vfork()
	{
		Process& child = NewProcess( *this );
		
		child.get_process().unshare_per_fork();
		
		// suspend parent for vfork
		
		itsForkedChildPID = child.GetPID();
		
		mark_vfork_stack_frame();
		
		// activate child
		
		gCurrentProcess = &child;
		
		return child;
		
	}
	
	static void close_fd_on_exec( void* keep, int fd, vfs::file_descriptor& desc )
	{
		if ( desc.will_close_on_exec() )
		{
			desc.handle.reset();
		}
	}
	
	static void CloseMarkedFileDescriptors( relix::fd_map& fileDescriptors )
	{
		// Close file descriptors with close-on-exec flag.
		
		fileDescriptors.for_each( &close_fd_on_exec, NULL );
	}
	
	static void CheckProgramFile( const vfs::node& programFile )
	{
		struct ::stat sb;
		
		stat( programFile, sb );
		
		if ( S_ISDIR( sb.st_mode ) )
		{
			p7::throw_errno( EISDIR );
		}
		
		if ( (sb.st_mode & S_IXUSR) == 0 )
		{
			p7::throw_errno( EACCES );
		}
	}
	
	static inline std::size_t minimum_stack_size()
	{
		return (CONFIG_MINI ? 32 : 64) * 1024;
	}
	
	class thing_that_may_resume_after_vfork
	{
		private:
			pid_t its_ppid;
		
		public:
			thing_that_may_resume_after_vfork() : its_ppid( 0 )
			{
			}
			
			void enable( pid_t ppid )
			{
				its_ppid = ppid;
			}
			
			~thing_that_may_resume_after_vfork();
	};
	
	thing_that_may_resume_after_vfork::~thing_that_may_resume_after_vfork()
	{
		if ( its_ppid )
		{
			GetProcess( its_ppid ).ResumeAfterFork();
		}
	}
	
	static void thread_switch_in( void* param )
	{
		Process& thread = *(Process*) param;
		
		gCurrentProcess = &thread;
		
		thread.switch_in();
	}
	
	static void thread_switch_out( void* param )
	{
		Process& thread = *(Process*) param;
		
		thread.switch_out();
		
		gCurrentProcess = NULL;
	}
	
	static relix::os_thread_box new_thread( Process& task )
	{
		const std::size_t min_stack = minimum_stack_size();
		
		return new_os_thread( &Process::thread_start,
		                      &task,
		                      min_stack,
		                      &thread_switch_in,
		                      &thread_switch_out );
	}
	
	void Process::Exec( const char*         path,
	                    const char* const   argv[],
	                    const char* const*  envp )
	{
		// Declare this first so it goes out of scope last
		thing_that_may_resume_after_vfork resume;
		
		relix::os_thread_box looseThread;
		
		vfs::node_ptr cwd = getcwd( get_process() );
		
		vfs::node_ptr programFile = resolve_pathname( *relix::root(), path, *cwd );
		
		vfs::resolve_links_in_place( *relix::root(), programFile );
		
		CheckProgramFile( *programFile );
		
		// Do we take the name before or after normalization?
		get_process().set_name( programFile->name() );
		
		ExecContext context( *programFile, argv );
		
		Normalize( path, context, *cwd );
		
		CloseMarkedFileDescriptors( FileDescriptors() );
		
		clear_signals_pending();
		
		get_process().reset_signal_handlers();
		
		// We always spawn a new thread for the exec'ed process.
		// If we've forked, then the thread is null, but if not, it's the
		// current thread -- be careful!
		
		// Create the new thread
		looseThread = new_thread( *this );
		
		relix::process& proc = get_process();
		
		// Save the process image that we're running from and set the new one.
		boost::intrusive_ptr< relix::process_image > old_image = &proc.get_process_image();
		
		proc.set_process_image( *new_process_image( *context.executable,
		                                            &context.argVector.front(),
		                                            envp ) );
		
		// Make the new thread belong to this process and save the old one
		swap_os_thread( looseThread );
		
		// Lose the current process image.  If we're not vforked and the
		// execution unit isn't cached, it's now gone.  But that's okay
		// since the thread terminates in execve().
		old_image.reset();
		
		itsLifeStage       = kProcessLive;
		itsSchedule        = kProcessRunning;  // a new process is runnable
		
		if ( gCurrentProcess != this )
		{
			return;
		}
		
		if ( looseThread.get() == 0 )
		{
			resume.enable( GetPPID() );
		}
	}
	
	void Process::Reexec( Reexec_Function f, void* _1,
	                                         void* _2,
	                                         void* _3,
	                                         void* _4,
	                                         void* _5,
	                                         void* _6,
	                                         void* _7 )
	{
		thing_that_may_resume_after_vfork resume;
		
		relix::os_thread_box looseThread = SpawnThread( (Clone_Function) f, _1 );
		
	//	itsReexecArgs[0] = (void*) f;
	//	itsReexecArgs[1] = _1;
		itsReexecArgs[2] = _2;
		itsReexecArgs[3] = _3;
		itsReexecArgs[4] = _4;
		itsReexecArgs[5] = _5;
		itsReexecArgs[6] = _6;
		itsReexecArgs[7] = _7;
		
		CloseMarkedFileDescriptors( FileDescriptors() );
		
		if ( gCurrentProcess != this )
		{
			return;
		}
		
		if ( looseThread.get() == 0 )
		{
			resume.enable( GetPPID() );
		}
	}
	
	relix::os_thread_box Process::SpawnThread( Clone_Function f, void* arg )
	{
		itsReexecArgs[0] = (void*) f;
		itsReexecArgs[1] = arg;
		itsReexecArgs[2] = 
		itsReexecArgs[3] =
		itsReexecArgs[4] =
		itsReexecArgs[5] =
		itsReexecArgs[6] =
		itsReexecArgs[7] = NULL;
		
		clear_signals_pending();
		
		// Create the new thread
		relix::os_thread_box looseThread = new_thread( *this );
		
		// Make the new thread belong to this process and save the old one
		swap_os_thread( looseThread );
		
		itsLifeStage       = kProcessLive;
		itsSchedule        = kProcessRunning;  // a new process is runnable
		
		return looseThread;
	}
	
	pid_t Process::GetPPID() const
	{
		return get_process().getppid();
	}
	
	pid_t Process::GetPGID() const
	{
		return get_process().get_process_group().id();
	}
	
	pid_t Process::GetSID()  const
	{
		return get_process().get_process_group().get_session().id();
	}
	
	char Process::run_state_code() const
	{
		if ( itsLifeStage == kProcessReleased )
		{
			return 'X';
		}
		
		if ( CountAsyncOps() > 0 )
		{
			return 'D';
		}
		
		if ( itsForkedChildPID != 0 )
		{
			return 'V';
		}
		
		if ( is_stopped() )
		{
			return 'T';
		}
		
		if ( relix::is_os_thread_stopped( get_os_thread() ) )
		{
			return 'W';
		}
		
		switch ( itsSchedule )
		{
			case kProcessRunning:      return 'R';  // [1]
			case kProcessSleeping:     return 'S';  // [2]
			case kProcessUnscheduled:  return 'Z';  // set in Process::Terminate()
			
			// [1] set on parent in execve() after child.Exec()
			//     set on parent in _exit() if forked
			//     set in Yield() after YieldToAnyThread() returns
			//     set in StopThread() after SetThreadState() if same thread
			// [2] set at end of Process::Exec() (after creating new thread for child)
			//     set in Process::Continue() if thread was stopped
			//     set in Yield() before YieldToAnyThread() is called
			
			default:
				return '?';
		}
	}
	
	relix::fd_map& Process::FileDescriptors()
	{
		return get_process().get_process_resources().get_fd_map();
	}
	
	void Process::ResumeAfterFork()
	{
		ASSERT( itsForkedChildPID != 0 );
		
		const int depth = 4 + TARGET_CPU_68K;
		
		using recall::get_stack_frame_pointer;
		
		typedef recall::stack_frame_pointer fp_t;
		
		fp_t vfork_fp = (fp_t) get_vfork_frame_pointer(       );
		fp_t stack_fp =        get_stack_frame_pointer( depth );
		
		// Stack grows down
		const bool stack_fault = !CONFIG_SYSCALL_STACKS  &&  stack_fp >= vfork_fp;
		
		Resume();
		
		pid_t child = itsForkedChildPID;
		
		itsForkedChildPID = 0;
		
		if ( itsPID == 1 )
		{
			itsSchedule = kProcessSleeping;
			
			return;
		}
		
		if ( stack_fault )
		{
			relix::deliver_fatal_signal( SIGSTKFLT );
		}
		
		relix::leave_system();
		
		resume_vfork( child );
	}
	
	void Process::fork_and_exit( int exit_status )
	{
		Process& parent = *this;
		
		Process& child = NewProcess( parent );
		
		child.itsLifeStage       = kProcessLive;
		
		child.swap_os_thread( parent );
		
		exit_process( parent, exit_status );
		
		child.Resume();
	}
	
	struct notify_param
	{
		pid_t  pid;
		bool   is_session_leader;
	};
	
	void* Process::notify_process( void* param, pid_t, Process& process )
	{
		const notify_param& pb = *(notify_param*) param;
		
		const pid_t that_pid = process.GetPID();
		
		if ( that_pid == pb.pid  &&  process.gettid() != that_pid )
		{
			// This is one of our threads; kill it.
			
			if ( process.itsLifeStage < kProcessTerminating )
			{
				process.Terminate();  // singly recursive call
			}
			
			if ( process.itsLifeStage == kProcessZombie )
			{
				process.Release();
			}
		}
		
		if ( pb.is_session_leader  &&  process.GetSID() == pb.pid )
		{
			process.Raise( SIGHUP );
		}
		
		if ( process.GetPPID() == pb.pid )
		{
			process.Orphan();
		}
		
		return NULL;
	}
	
	const struct sigaction& Process::GetSignalAction( int signo ) const
	{
		ASSERT( signo >    0 );
		ASSERT( signo < NSIG );
		
		return get_process().get_sigaction( signo );
	}
	
	// This function doesn't return if the process is current.
	void Process::Terminate()
	{
		if ( WCOREDUMP( itsResult )  &&  itMayDumpCore )
		{
			// prevent reentry if backtrace causes exception
			itMayDumpCore = false;
			
			DumpBacktrace();
		}
		
		itsLifeStage = kProcessTerminating;
		itsSchedule  = kProcessUnscheduled;
		
		pid_t ppid = GetPPID();
		pid_t pid  = GetPID();
		pid_t sid  = GetSID();
		
		bool isSessionLeader = pid == sid;
		
		if ( gettid() == pid )
		{
			get_process().reset_process_resources();
		}
		
		itsLifeStage = kProcessZombie;
		
		if ( gettid() == pid )
		{
			Process& parent = GetProcess( ppid );
			
			if ( ppid > 1  &&  waits_for_children( parent.get_process() ) )
			{
				parent.Raise( SIGCHLD );
			}
			else
			{
				Release();
			}
			
			notify_param param = { pid, isSessionLeader };
			
			for_each_process( &notify_process, &param );
		}
		
		if ( gCurrentProcess != this )
		{
			return;
		}
		
		relix::restore_the_A5_world();
		
		/*
			For a vforked process (with null thread) this does nothing.
			Otherwise, reset() is safe because it swaps with a temporary
			before destroying the thread (so the copy that doesn't get
			nulled out when the thread terminates is on the stack).
			
			Mark the thread inactive now, in case it doesn't get joined
			right away.  If the OS thread still lives after reset, mark
			the thread active again.
		*/
		
		relix::mark_thread_inactive( gettid() );
		
		reset_os_thread();
		
		relix::mark_thread_active( gettid() );
		
		// We get here if this is a vforked child, or fork_and_exit().
		
		if ( pid_t ppid = GetPPID() )
		{
			Process& parent = GetProcess( ppid );
			
			if ( parent.itsForkedChildPID != 0 )
			{
				parent.ResumeAfterFork();  // Calls longjmp()
			}
		}
	}
	
	// This function doesn't return if the process is current.
	void Process::Terminate( int wait_status )
	{
		itsResult = wait_status;
		
		Terminate();
	}
	
	void Process::Orphan()
	{
		ASSERT( GetPPID() != 1 );
		
		get_process().orphan();
		
		if ( itsLifeStage == kProcessZombie )
		{
			Release();
		}
	}
	
	void Process::Release()
	{
		ASSERT( itsLifeStage == kProcessZombie );
		
		if ( gettid() == GetPID() )
		{
			get_process().clear_ppid();  // Don't match PPID comparisons
		}
		
		itsLifeStage = kProcessReleased;
		
		notify_reaper();
	}
	
	void Process::Resume()
	{
		gCurrentProcess = this;
		
		clear_stack_frame_mark();  // We don't track this while running
		
		itsSchedule = kProcessRunning;
	}
	
	void Process::Pause( ProcessSchedule newSchedule )
	{
		itsSchedule = newSchedule;
		
		mark_current_stack_frame();
		
		if ( newSchedule == kProcessStopped )
		{
			relix::stop_os_thread( get_os_thread() );
		}
		else
		{
			relix::os_thread_yield();
		}
		
		Resume();
	}
	
	// Doesn't return if the process was current and receives a fatal signal while stopped.
	// But always returns when *raising* a fatal signal.
	void Process::Raise( int signo )
	{
		if ( itsLifeStage >= kProcessTerminating  ||  itsResult != 0 )
		{
			return;
		}
		
		if ( GetPID() == 1 )
		{
			return;
		}
		
		typedef void (*signal_handler_t)(int);
		
		signal_handler_t action = GetSignalAction( signo ).sa_handler;
		
		if ( action == SIG_IGN )
		{
			return;
		}
		
		set_pending_signal( signo );
		
		if ( signo == SIGKILL  ||  signo == SIGCONT )
		{
			Continue();
		}
	}
	
	// This function doesn't return if the process receives a fatal signal.
	bool Process::HandlePendingSignals( bool may_throw )
	{
		if ( itsLifeStage > kProcessLive )
		{
			return false;  // Don't try to handle signals in terminated processes
		}
		
		if ( get_process().get_alarm_clock().check() )
		{
			Raise( SIGALRM );
		}
		
		if ( itsResult != 0 )
		{
			// Fatal signal received.  Terminate.
			
			Terminate();  // Kills the thread
			
			// Not reached
		}
		
		return DeliverPendingSignals( may_throw );
	}
	
	bool Process::DeliverPendingSignals( bool may_throw )
	{
		bool signal_was_caught = false;
		
		for ( int signo = 1;  signo < NSIG;  ++signo )
		{
			const sigset_t active_signals = signals_pending() & ~signals_blocked();
			
			if ( !active_signals )
			{
				return false;
			}
			
			const sigset_t signo_mask = 1 << (signo - 1);
			
			if ( active_signals & signo_mask )
			{
				const struct sigaction& action = GetSignalAction( signo );
				
				if ( action.sa_handler == SIG_IGN )
				{
					continue;
				}
				
				relix::signal signal = { signo, action };
				
				if ( action.sa_handler == SIG_DFL )
				{
					using namespace relix;
					
					const signal_traits traits = global_signal_traits[ signo ];
					
					switch ( traits & signal_default_action_mask )
					{
						case signal_discard:
							break;
						
						case signal_terminate:
							signal.number |= traits & signal_core;
							
							throw signal;
						
						case signal_stop:
							Stop();
							break;
						
						case signal_continue:
							Continue();
							break;
					}
					
					continue;
				}
				
				signal_was_caught = true;
				
				if ( !may_throw )
				{
					continue;
				}
				
				if ( action.sa_flags & SA_RESETHAND  &&  signo != SIGILL  &&  signo != SIGTRAP )
				{
					const struct sigaction default_sigaction = { SIG_DFL };
					
					get_process().set_sigaction( signo, default_sigaction );
				}
				
				throw signal;
			}
		}
		
		return signal_was_caught;
	}
	
	// Stops the process' thread.  Eventually returns.
	void Process::Stop()
	{
		ASSERT( gCurrentProcess == this );
		
		relix::mark_thread_inactive( gettid() );
		
		Pause( kProcessStopped );
		
		relix::mark_thread_active( gettid() );
	}
	
	void Process::Continue()
	{
		relix::os_thread_id thread = get_os_thread();
		
		if ( thread == 0 )
		{
			relix::console::log( STR_LEN( "Genie: Process::Continue(): no thread assigned\n" ) );
			
			return;
		}
		
		if ( itsForkedChildPID != 0 )
		{
			return;
		}
		
		if ( relix::is_os_thread_stopped( thread ) )
		{
			itsSchedule = kProcessSleeping;
			
			relix::wake_os_thread( thread );
		}
	}
	
	
	void Process::Breathe()
	{
		Pause( kProcessRunning );
	}
	
	void Process::Yield()
	{
		Pause( kProcessSleeping );
	}
	
	void Process::AsyncYield()
	{
		++itsAsyncOpCount;
		
		Yield();
		
		--itsAsyncOpCount;
	}
	
	// declared in Process/AsyncYield.hh
	void AsyncYield()
	{
		if ( gCurrentProcess == NULL )
		{
			relix::os_thread_yield();
		}
		else
		{
			gCurrentProcess->AsyncYield();
		}
	}
	
}

namespace relix
{
	
	process_group* get_process_group( pid_t pgid )
	{
		using namespace Genie;
		
		return (process_group*) for_each_process( &find_process_group, &pgid );
	}
	
	void signal_process_group( int signo, pid_t pgid )
	{
		if ( pgid != no_pgid )
		{
			Genie::SendSignalToProcessGroup( signo, pgid );
		}
	}
	
	void terminate_current_process( int wait_status )
	{
		Genie::gCurrentProcess->Terminate( wait_status );
	}
	
}
