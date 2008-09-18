/*	=========
 *	A-line.cc
 *	=========
 */

#include "A-line/A-line.hh"

// Standard C++
#include <algorithm>
#include <functional>
#include <map>
#include <numeric>
#include <vector>

// Standard C/C++
#include <cstdio>
#include <cstring>

// Standard C
#include "stdlib.h"

// POSIX
#include "fcntl.h"
#include "sys/wait.h"
#include "unistd.h"

// Iota
#include "iota/strings.hh"

// Nucleus
#include "Nucleus/NAssert.h"

// POSeven
#include "POSeven/Errno.hh"
#include "POSeven/FileDescriptor.hh"
#include "POSeven/Open.hh"
#include "POSeven/Pathnames.hh"
#include "POSeven/Stat.hh"
#include "POSeven/functions/vfork.hh"
#include "POSeven/functions/waitpid.hh"
#include "POSeven/functions/_exit.hh"

// Nitrogen Extras / Utilities
//#include "Utilities/Processes.h"

// Orion
#include "Orion/GetOptions.hh"
#include "Orion/Main.hh"

// CompileDriver
#include "CompileDriver/ProjectCatalog.hh"
#include "CompileDriver/ProjectConfig.hh"
#include "CompileDriver/ProjectDotConf.hh"

// A-line
#include "A-line/Commands.hh"
#include "A-line/Exceptions.hh"
#include "A-line/Compile.hh"
#include "A-line/Link.hh"
#include "A-line/Locations.hh"
#include "A-line/Project.hh"
#include "A-line/ProjectCommon.hh"
#include "A-line/TargetNames.hh"
#include "A-line/Task.hh"
#include "CompileDriver/ProjectCatalog.hh"
#include "CompileDriver/ProjectConfig.hh"


namespace tool
{
	
	namespace p7 = poseven;
	namespace O = Orion;
	
	
	using namespace io::path_descent_operators;
	
	
	static bool gDryRun = false;
	
	static OptionsRecord gOptions;
	
	OptionsRecord& Options()
	{
		return gOptions;
	}
	
	/*
	static void SwapFrontProcess( const ProcessSerialNumber& from, const ProcessSerialNumber& to )
	{
		try
		{
			if ( N::SameProcess( from, N::GetFrontProcess() ) )
			{
				N::SetFrontProcess( to );
			}
		}
		catch ( ... )
		{
		}
	}
	
	class ApplicationLayerSwitch
	{
		private:
			ProcessSerialNumber itsTargetApp;
			
			ApplicationLayerSwitch           ( const ApplicationLayerSwitch& );
			ApplicationLayerSwitch& operator=( const ApplicationLayerSwitch& );
		
		public:
			ApplicationLayerSwitch() : itsTargetApp( N::CurrentProcess() )  {}
			
			ApplicationLayerSwitch( N::OSType signature ) : itsTargetApp( NX::LaunchApplication( signature ) )
			{
				SwapFrontProcess( N::CurrentProcess(), itsTargetApp );
			}
			
			~ApplicationLayerSwitch()
			{
				SwapFrontProcess( itsTargetApp, N::CurrentProcess() );
			}
			
			void Switch( const ProcessSerialNumber& newTarget )
			{
				SwapFrontProcess( itsTargetApp, newTarget );
				
				itsTargetApp = newTarget;
			}
			
			void Switch( N::OSType newTarget )
			{
				Switch( NX::LaunchApplication( newTarget ) );
			}
	};
	*/
	
	
	inline std::string MakeTargetName( const TargetInfo& info )
	{
		return MakeTargetName( info.platform & archMask,
		                       info.platform & runtimeMask,
		                       info.platform & apiMask,
		                       info.build );
	}
	
	
	static std::string ShellEscapedWord( const std::string& word )
	{
		std::string result;
		
		result.reserve( word.size() );
		
		for ( const char* p = word.c_str();  *p != '\0';  ++p )
		{
			if ( !std::isalnum( *p )  &&  *p != '_' )
			{
				switch ( *p )
				{
					case '_':
					case '+':
					case '-':
					case '/':
					case '.':
					case ',':
					case '@':
					case '%':
						// shell-safe character; don't escape
						break;
					
					default:
						// insert a backslash before an unsafe character
						result += '\\';
						break;
				}
			}
			
			result += *p;
		}
		
		return result;
	}
	
	static void PrintCommand( const std::vector< const char* >& command )
	{
		//ASSERT( command.size() > 1 ) );
		
		std::string command_line = command.front();
		
		typedef std::vector< const char* >::const_iterator Iter;
		
		Iter end = command.end() - (command.back() == NULL ? 1 : 0);
		
		for ( Iter it = command.begin() + 1;  it != end;  ++it )
		{
			command_line += ' ';
			command_line += *it;
		}
		
		command_line += "\n";
		
		p7::write( p7::stdout_fileno, command_line.data(), command_line.size() );
	}
	
	static void PrintCommandForShell( const std::vector< const char* >& command )
	{
		if ( gOptions.verbose || gDryRun )
		{
			PrintCommand( command );
		}
	}
	
	static void SetEditorSignature( const char* filename )
	{
		if ( const char* editor_sig = getenv( "MAC_EDITOR_SIGNATURE" ) )
		{
			const char* command[] = { "/Developer/Tools/SetFile", "-t", "TEXT", "-c", editor_sig, filename, NULL };
			
			p7::pid_t pid = POSEVEN_VFORK();
			
			if ( pid == 0 )
			{
				(void) execv( command[0], (char**) command );
				
				_exit( 127 );
			}
			
			(void) p7::waitpid( pid );
		}
	}
	
	static bool open_diagnostics_file( const char* path )
	{
		int fd = ::open( path, O_WRONLY | O_CREAT | O_TRUNC, 0666 );
		
		if ( fd < 0 )
		{
			return false;
		}
		
		dup2( fd, STDERR_FILENO );
		
		close( fd );
		
		return true;
	}
	
	inline bool is_null( const char* string )
	{
		return string == NULL  ||  string[0] == '\0';
	}
	
	
	static p7::pid_t launch_job( const std::vector< const char* >& command, const char* diagnostics_path )
	{
		p7::pid_t pid = POSEVEN_VFORK();
		
		if ( pid == 0 )
		{
			if ( !is_null( diagnostics_path ) && !open_diagnostics_file( diagnostics_path ) )
			{
				std::perror( diagnostics_path );
				
				p7::_exit( p7::exit_failure );
			}
			
			execvp( command.front(), const_cast< char** >( &command.front() ) );
			
			_exit( 127 );
		}
		
		return pid;
	}
	
	static void report_diagnostics( size_t size, bool had_errors, const char* diagnostics_path )
	{
		const char* stuff = had_errors ? "errors" : "warnings";
		
		char path_buffer[ 4096 ];
		
		const char* pathname = realpath( diagnostics_path, path_buffer );
		
		if ( pathname == NULL )
		{
			pathname = diagnostics_path;
		}
		
		std::fprintf( stderr, "#\n# %d bytes of %s\n#\n" "    report %s\n#\n",
		                            size,       stuff,               pathname );
	}
	
	void check_diagnostics( bool succeeded, const char* diagnostics_path )
	{
		if ( !is_null( diagnostics_path ) )
		{
			struct ::stat stat_buffer = p7::stat( diagnostics_path );
			
			if ( const size_t size = stat_buffer.st_size )
			{
				report_diagnostics( size, !succeeded, diagnostics_path );
				
				return;
			}
			
			// empty file; delete, ignore errors
			(void) unlink( diagnostics_path );
		}
	}
	
	
	static std::map< p7::pid_t, TaskPtr > global_running_tasks;
	
	
	inline bool is_user_break( p7::wait_t wait_status )
	{
		return p7::wifexited( wait_status )  &&  p7::wexitstatus( wait_status ) == 128;
	}
	
	inline bool is_plain_error( p7::wait_t wait_status )
	{
		return p7::wifexited( wait_status )  &&  p7::wexitstatus( wait_status ) == 1;
	}
	
	static void end_task( p7::pid_t pid, p7::wait_t wait_status )
	{
		std::map< p7::pid_t, TaskPtr >::iterator it = global_running_tasks.find( pid );
		
		ASSERT( it != global_running_tasks.end() );
		
		TaskPtr task = it->second;
		
		global_running_tasks.erase( it );
		
		if ( wait_status == 0 )
		{
			task->Success();
		}
		else if ( is_plain_error( wait_status ) )
		{
			task->Failure();
		}
		else
		{
			const bool signaled = p7::wifsignaled( wait_status );
			
			const char* ended  = signaled ? "terminated via signal" : "exited with status";
			int         status = signaled ? WTERMSIG( wait_status ) : WEXITSTATUS( wait_status );
			
			std::fprintf( stderr, "The last command %s %d.  Aborting.\n", ended, status );
			
			O::ThrowExitStatus( signaled ? 2 : p7::exit_failure );
		}
	}
	
	static bool wait_and_end_task( bool nonblocking )
	{
		p7::wait_t wait_status;
		
		if ( p7::pid_t pid = p7::waitpid( p7::pid_t( -1 ), wait_status, nonblocking ? WNOHANG : 0 ) )
		{
			if ( is_user_break( wait_status ) )
			{
				p7::write( p7::stderr_fileno, STR_LEN( "### Aborting on user break via ToolServer.\n" ) );
				
				O::ThrowExitStatus( 128 );
			}
			
			end_task( pid, wait_status );
			
			return true;
		}
		
		return false;
	}
	
	static bool reap_job( bool nonblocking )
	{
		return !global_running_tasks.empty() && wait_and_end_task( nonblocking );
	}
	
	static void reap_jobs( bool nonblocking )
	{
		while ( reap_job( nonblocking ) )
		{
			continue;
		}
	}
	
	static std::size_t global_job_limit = 1;
	
	static void wait_for_jobs()
	{
		// If all the available slots are taken, wait for a job to exit
		while ( global_running_tasks.size() >= global_job_limit )
		{
			wait_and_end_task( false );
		}
		
		reap_jobs( true );
	}
	
	void ExecuteCommand( const TaskPtr&                     task,
	                     const std::string&                 caption,
	                     const std::vector< const char* >&  command,
	                     const char*                        diagnostics_file_path )
	{
		ASSERT( command.size() > 1 );
		ASSERT( command.back() == NULL );
		
		std::printf( "%s\n", caption.c_str() );
		
		PrintCommandForShell( command );
		
		if ( gDryRun )
		{
			return;
		}
		
		/*
		ApplicationLayerSwitch activateToolServerForCommand;
		
		bool shouldSwitchLayers = !TARGET_RT_MAC_MACHO  &&  (command.substr( 0, 6 ) == "tlsrvr"  ||  command.substr( 0, 4 ) == "mwcc" );
		
		if ( shouldSwitchLayers )
		{
			const N::OSType sigToolServer = N::OSType( 'MPSX' );
			
			//static ApplicationLayerSwitch activateToolServerForSession( sigToolServer );
			
			activateToolServerForCommand.Switch( sigToolServer );
		}
		*/
		
		const bool has_diagnostics_file = !is_null( diagnostics_file_path );
		
		if ( has_diagnostics_file )
		{
			std::string diagnostics_dir = io::get_preceding_directory( diagnostics_file_path );
			
			mkdir_path( diagnostics_dir );
		}
		
		p7::pid_t pid = launch_job( command, diagnostics_file_path );
		
		global_running_tasks[ pid ] = task;
		
		if ( has_diagnostics_file )
		{
			SetEditorSignature( diagnostics_file_path );
		}
		
		wait_for_jobs();
	}
	
	
	class project_builder
	{
		private:
			const TargetInfo& its_target_info;
		
		public:
			project_builder( const TargetInfo& info ) : its_target_info( info )
			{
			}
			
			void operator()( const std::string& project_name ) const;
	};
	
	void project_builder::operator()( const std::string& project_name ) const
	{
		Project& project = GetProject( project_name, its_target_info.platform );
		
		bool needToBuild = ProductGetsBuilt( project.Product() );
		
		if ( needToBuild )
		{
			TaskPtr project_base_task( new NullTask() );
			TaskPtr source_dependency( new NullTask() );
			
			std::vector< TaskPtr > tool_dependencies;
			
			CompileSources( project, its_target_info, project_base_task, source_dependency, tool_dependencies );
			LinkProduct   ( project, its_target_info, project_base_task, source_dependency, tool_dependencies );
			
			AddReadyTask( project_base_task );
		}
	}
	
	static void BuildTarget( const Project& project, const TargetInfo& targetInfo )
	{
		std::string targetName = MakeTargetName( targetInfo );
		
		std::string targetDir = TargetDirPath( targetName );
		
		PrintCommandForShell( MakeCommand( "cd", targetDir.c_str() ) );
		
		chdir( targetDir.c_str() );
		
		const std::vector< std::string >& prereqs = project.AllUsedProjects();
		std::for_each
		(
			prereqs.begin(), 
			prereqs.end(), 
			project_builder( targetInfo )
		);
	}
	
	static void ApplyTargetDefaults( TargetInfo& target )
	{
		ApplyPlatformDefaults( target.platform );
		
		if ( target.build == buildDefault )
		{
			target.build = buildDebug;
		}
		
		if ( !ALINE_CROSS_DEVELOPMENT  ||  target.platform & runtimeMachO )
		{
			target.envType   = envUnix;
		}
		else
		{
			target.envType   = envLamp;
		}
		
		// (a) Metrowerks/Mach-O is fully untested and almost certainly broken
		// (b) This doesn't consider MPW compilers
		
		if ( !ALINE_CROSS_DEVELOPMENT  ||  target.platform & runtimeMachO )
		{
			target.toolchain = toolchainGNU;
		}
		else
		{
			target.toolchain = toolchainMetrowerks;
		}
	}
	
	static TargetInfo MakeTargetInfo( const Project& project, Platform platform, BuildVariety build )
	{
		TargetInfo targetInfo( platform, build );
		
		//targetInfo.DeduceFromProduct( project.Product() );
		//targetInfo.ApplyOptions( gOptions.target );
		
		ApplyTargetDefaults( targetInfo );
		
		return targetInfo;
	}
	
	int Main( int argc, iota::argv_t argv )
	{
		if ( argc <= 1 )  return 0;
		
		Platform arch    = platformUnspecified;
		Platform runtime = platformUnspecified;
		Platform macAPI  = platformUnspecified;
		
		BuildVariety buildVariety = buildDefault;
		
		// General
		
		O::BindOption( "-v", gOptions.verbose );
		
		O::AliasOption( "-v", "--verbose" );
		
		// Actions
		
		O::BindOption( "-a", gOptions.all );
		
		O::AliasOption( "-a", "--all" );
		
		O::BindOption( "-n", gDryRun );
		
		O::AliasOption( "-n", "--dry-run" );
		
		O::BindOption( "-t", gOptions.catalog );
		
		O::AliasOption( "-t", "--catalog" );
		
		// Targeting
		
		O::BindOption( "-6", arch, arch68K );
		O::BindOption( "-P", arch, archPPC );
		O::BindOption( "-8", arch, archX86 );
		
		O::BindOption( "-4", runtime, runtimeA4CodeResource );
		O::BindOption( "-5", runtime, runtimeA5CodeSegments );
		O::BindOption( "-F", runtime, runtimeCodeFragments  );
		O::BindOption( "-O", runtime, runtimeMachO          );
		
		O::BindOption( "-B", macAPI, apiMacBlue   );
		O::BindOption( "-C", macAPI, apiMacCarbon );
		
		O::BindOption( "-g", buildVariety, buildDebug   );
		O::BindOption( "-R", buildVariety, buildRelease );
		O::BindOption( "-D", buildVariety, buildDemo    );
		
		O::AliasOption( "-6", "--68k" );
		O::AliasOption( "-P", "--ppc" );
		O::AliasOption( "-8", "--x86" );
		
		O::AliasOption( "-4", "--a4"    );
		O::AliasOption( "-5", "--a5"    );
		O::AliasOption( "-F", "--cfm"   );
		O::AliasOption( "-O", "--macho" );
		
		O::AliasOption( "-B", "--blue"   );
		O::AliasOption( "-C", "--carbon" );
		
		O::AliasOption( "-g", "--debug"   );
		O::AliasOption( "-R", "--release" );
		O::AliasOption( "-D", "--demo"    );
		
		// Performance
		
		O::BindOption( "-j", global_job_limit );
		
		O::GetOptions( argc, argv );
		
		char const *const *freeArgs = O::FreeArguments();
		
		Platform targetPlatform = arch | runtime | macAPI;
		
		AddPendingSubproject( UserSrcTreePath() );
		
		std::string catalog_cache_pathname = get_user_cache_pathname() / "catalog";
		
		bool cache_was_written = false;
		
		if ( gOptions.catalog || !io::file_exists( catalog_cache_pathname ) )
		{
			p7::write( p7::stdout_fileno, STR_LEN( "# Catalogging project configs..." ) );
			
			while ( AddPendingSubprojects() )
			{
				continue;
			}
			
			write_catalog_cache( p7::open( catalog_cache_pathname,
			                               p7::o_wronly | p7::o_creat | p7::o_trunc,
			                               0644 ) );
			
			cache_was_written = true;
			
			p7::write( p7::stdout_fileno, STR_LEN( "done\n" ) );
		}
		else
		{
			read_catalog_cache( p7::open( catalog_cache_pathname, p7::o_rdonly ) );
		}
		
		p7::write( p7::stdout_fileno, STR_LEN( "# Loading project data..." ) );
		
		ApplyPlatformDefaults( targetPlatform );
		
		for ( int i = 0;  freeArgs[ i ] != NULL;  ++i )
		{
			const std::string& project_name = freeArgs[ i ];
			
			try
			{
				Project& project = GetProject( project_name, targetPlatform );
			}
			catch ( const NoSuchProject& )
			{
				std::fprintf( stderr, "A-line: No such project '%s'\n", project_name.c_str() );
				
				if ( !cache_was_written )
				{
					std::fprintf( stderr, "%s\n", "A-line: (use 'A-line -t' to refresh the project catalog)" );
				}
				
				return EXIT_FAILURE;
			}
			catch ( const NoSuchUsedProject& ex )
			{
				std::fprintf( stderr, "A-line: No such project '%s' used by %s\n",
				                                                ex.used.c_str(),
				                                                            ex.projName.c_str() );
				
				if ( !cache_was_written )
				{
					std::fprintf( stderr, "%s\n", "A-line: (use 'A-line -t' to refresh the project catalog)" );
				}
				
				return EXIT_FAILURE;
			}
			catch ( const p7::errno_t& err )
			{
				std::fprintf( stderr, "A-line: %s: %s\n", project_name.c_str(), std::strerror( err ) );
				
				throw;
			}
		}
		
		p7::write( p7::stdout_fileno, STR_LEN( "done.\n" ) );
		
		p7::write( p7::stdout_fileno, STR_LEN( "# Generating task graph..." ) );
		
		TargetInfo target_info( targetPlatform, buildVariety );
		
		ApplyTargetDefaults( target_info );
		
		for ( int i = 0;  freeArgs[ i ] != NULL;  ++i )
		{
			const std::string& proj = freeArgs[ i ];
			
			try
			{
				Project& project = GetProject( proj, targetPlatform );
				
				BuildTarget( project, target_info );
			}
			catch ( const p7::errno_t& err )
			{
				std::fprintf( stderr, "A-line: %s: %s\n", proj.c_str(), std::strerror( err ) );
				
				throw;
			}
		}
		
		p7::write( p7::stdout_fileno, STR_LEN( "done.\n" ) );
		
		while ( StartNextTask() || reap_job( false ) )
		{
			continue;
		}
		
		reap_jobs( false );
		
		if ( std::size_t n = CountFailures() )
		{
			std::fprintf( stderr, "###\n"
			                      "### A-line: %d task%s had errors\n"
			                      "###\n",     n,     n == 1 ? ""
			                                                 : "s" );
			
			return EXIT_FAILURE;
		}
		
		return EXIT_SUCCESS;
	}
	
}

