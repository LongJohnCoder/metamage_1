/*	=======
 *	wait.cc
 *	=======
 */

// Standard C
#include <errno.h>

// POSIX
#include "sys/wait.h"

// poseven
#include "poseven/types/errno_t.hh"

// Genie
#include "Genie/current_process.hh"
#include "Genie/ProcessList.hh"
#include "Genie/SystemCallRegistry.hh"
#include "Genie/api/yield.hh"


namespace Genie
{
	
	namespace p7 = poseven;
	
	
	struct wait_param
	{
		pid_t  ppid;
		pid_t  pgid;
		bool   match_untraced;
		bool   has_children;
	};
	
	static void* check_process( void* param, pid_t pid, Process& process )
	{
		wait_param& pb = *(wait_param*) param;
		
		if ( process.gettid() != process.GetPID() )
		{
			return NULL;  // ignore non-leader threads
		}
		
		const bool is_child     =                   process.GetPPID() == pb.ppid;
		const bool pgid_matches = pb.pgid == 0  ||  process.GetPGID() == pb.pgid;
		
		const bool terminated   = process.GetLifeStage() == kProcessZombie;
		
		const bool stopped      = process.GetSchedule() == kProcessStopped;
		
		const bool traced       = process.IsBeingTraced();
		
		if ( is_child && pgid_matches )
		{
			if ( terminated  ||  (stopped && (traced || pb.match_untraced)) )
			{
				return &process;
			}
			
			pb.has_children = true;
		}
		
		return NULL;
	}
	
	static Process* CheckAny( pid_t ppid, pid_t pid, bool match_untraced )
	{
		pid_t pgid = pid == -1 ? 0
		           : pid ==  0 ? CurrentProcess().GetPGID()
		           :             -pid;
		
		wait_param param = { ppid, pgid, match_untraced, false };
		
		if ( Process* process = (Process*) for_each_process( &check_process, &param ) )
		{
			return process;
		}
		
		if ( !param.has_children )
		{
			p7::throw_errno( ECHILD );
		}
		
		return NULL;
	}
	
	static Process* CheckPID( pid_t ppid, pid_t pid, bool match_untraced )
	{
		Process* process = FindProcess( pid );
		
		if ( process == NULL )
		{
			// No such process
			p7::throw_errno( ECHILD );
		}
		
		if ( process->GetPPID() != ppid )
		{
			// Process exists but its not your child
			p7::throw_errno( EINVAL );
		}
		
		bool terminated = process->GetLifeStage() == kProcessZombie;
		
		bool stopped    = process->GetSchedule() == kProcessStopped;
		
		bool traced     = process->IsBeingTraced();
		
		if ( terminated  ||  (stopped && (traced || match_untraced)) )
		{
			return process;
		}
		
		// Your child is still alive, please wait...
		return NULL;
	}
	
	static Process* CheckTID( pid_t caller, pid_t tid )
	{
		Process* thread = FindProcess( tid );
		
		if ( thread == NULL )
		{
			// No such thread
			p7::throw_errno( ECHILD );
		}
		
		if ( thread->GetPID() != caller )
		{
			// Thread exists but it's not in your thread group
			p7::throw_errno( EINVAL );
		}
		
		const bool terminated = thread->GetLifeStage() == kProcessZombie;
		
		if ( terminated )
		{
			return thread;
		}
		
		// The thread is still alive, please wait...
		return NULL;
	}
	
	static pid_t waitpid( pid_t pid, int* stat_loc, int options )
	{
		Process& caller = current_process();
		
		pid_t ppid = caller.GetPID();
		
		bool untraced = options & WUNTRACED;
		
		const bool is_thread = options & __WTHREAD;
		
		try
		{
			while ( true )
			{
				if ( Process* child = pid == -1 ? CheckAny( ppid, pid, untraced )
				                    : is_thread ? CheckTID( ppid, pid )
				                                : CheckPID( ppid, pid, untraced ) )
				{
					if ( stat_loc != NULL )
					{
						*stat_loc = child->GetSchedule() == kProcessStopped ? 0x7f : child->Result();
					}
					
					pid_t found_pid = child->GetPID();
					
					if ( child->GetLifeStage() == kProcessZombie )
					{
						caller.AccumulateChildTimes( child->GetTimes() );
						
						child->Release();
					}
					
					return found_pid;
				}
				
				if ( options & WNOHANG )
				{
					// no hang
					return 0;
				}
				
				try_again( false );
			}
		}
		catch ( ... )
		{
			return set_errno_from_exception();
		}
		
		// Not reached
		return -1;
	}
	
	#pragma force_active on
	
	REGISTER_SYSTEM_CALL( waitpid );
	
	#pragma force_active reset
	
}

