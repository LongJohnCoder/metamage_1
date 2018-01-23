/*	==========
 *	Process.hh
 *	==========
 */

#ifndef GENIE_PROCESS_HH
#define GENIE_PROCESS_HH

// relix-kernel
#include "relix/task/thread.hh"


struct sigaction;
struct tms;


namespace relix
{
	
	class fd_map;
	
}

namespace Genie
{
	
	class NotExecutableError {};
	
	
	enum ProcessLifeStage
	{
		kProcessStarting,
		kProcessLive,
		kProcessTerminating,
		kProcessZombie,
		kProcessReleased
	};
	
	enum ProcessSchedule
	{
		kProcessRunning,
		kProcessSleeping,
		kProcessStopped,
		kProcessUnscheduled
	};
	
	typedef int (*Reexec_Function)( void* _1,
	                                void* _2,
	                                void* _3,
	                                void* _4,
	                                void* _5,
	                                void* _6,
	                                void* _7 );
	
	typedef int (*Clone_Function)( void* arg );
	
	// Genie::Process is actually a thread, not a process.
	
	class Process : public relix::thread
	{
		private:
			pid_t itsPID;
			pid_t itsForkedChildPID;
			
			ProcessLifeStage        itsLifeStage;
			ProcessSchedule         itsSchedule;
			
			int itsResult;
			
			unsigned itsAsyncOpCount;
		
		private:
			void* itsReexecArgs[8];
			
			bool itMayDumpCore;
		
		private:
			// non-copyable
			Process           ( const Process& );
			Process& operator=( const Process& );
			
			void Resume();
			void Pause( ProcessSchedule newSchedule );
			
			bool DeliverPendingSignals( bool may_throw );
			
			void Terminate();
			
			static void* notify_process( void* param, pid_t, Process& process );
			
			void Orphan();
		
		public:
			bool IsBeingTraced() const  { return false; }
			
			void Terminate( int wait_status );
			
			struct RootProcess {};
			
			Process( RootProcess );
			Process( Process& parent, pid_t pid, pid_t tid );
			
			static void* thread_start( void* param, const void* bottom, const void* limit );
			
			bool MayDumpCore() const  { return itMayDumpCore; }
			
			void SuppressCoreDump()  { itMayDumpCore = false; }
			void AllowCoreDump   ()  { itMayDumpCore = true;  }
			
			pid_t gettid() const  { return id(); }
			
			pid_t GetPPID() const;
			pid_t GetPID()  const  { return itsPID;  }
			
			pid_t GetPGID() const;
			pid_t GetSID()  const;
			
			ProcessLifeStage  GetLifeStage() const  { return itsLifeStage; }
			
			bool is_stopped() const  { return itsSchedule == kProcessStopped; }
			
			char run_state_code() const;
			
			int Result() const  { return itsResult; }
			
			void Release();
			
			void Breathe();
			void Yield();
			void AsyncYield();
			
			relix::fd_map& FileDescriptors();
			
			const struct sigaction& GetSignalAction( int signo ) const;
			
			void Raise( int signal );
			
			void ResumeAfterFork();
			
			void fork_and_exit( int exit_status );
		
		public:
			Process& vfork();
			
			void Exec( const char*         path,
			           const char* const   argv[],
			           const char* const*  envp );
			
			void Reexec( Reexec_Function f, void* _1,
			                                void* _2,
			                                void* _3,
			                                void* _4,
			                                void* _5,
			                                void* _6,
			                                void* _7 );
			
			relix::os_thread_box SpawnThread( Clone_Function f, void* arg );
			
			void Stop();
			void Continue();
			
			bool HandlePendingSignals( bool may_throw );
	};
	
	void SendSignalToProcessGroup( int sig, pid_t pgid );
	
	Process& CurrentProcess();
	
	Process& GetProcess( pid_t pid );
	
	Process* FindProcess( pid_t pid );
	
}

#endif
