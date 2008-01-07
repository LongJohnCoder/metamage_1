/*	=========
 *	ptrace.cc
 *	=========
 */

// POSIX
#include "signal.h"
#include "sys/ptrace.h"
#include "unistd.h"

// Genie
#include "Genie/Process.hh"
#include "Genie/Process/SavedRegisters.hh"
#include "Genie/SystemCallRegistry.hh"
#include "Genie/SystemCalls.hh"


namespace Genie
{
	
	DECLARE_MODULE_INIT( Kernel_ptrace )
	DEFINE_MODULE_INIT(  Kernel_ptrace )
	
	
	static int ptrace( int request, pid_t pid, void* addr, int data )
	{
		SystemCallFrame frame( "ptrace" );
		
		Process& current = frame.Caller();
		
		try
		{
			if ( request == PTRACE_TRACEME )
			{
				if ( current.IsBeingTraced() )
				{
					return frame.SetErrno( EPERM );
				}
				
				current.StartTracing( current.GetPPID() );
				
				return 0;
			}
			
			// Remaining requests specify target by pid
			
			if ( pid == 1 )
			{
				// Can't trace init
				return frame.SetErrno( EPERM );
			}
			
			Process& target = GetProcess( pid );
			
			if ( request == PTRACE_ATTACH )
			{
				if ( target.IsBeingTraced() )
				{
					return frame.SetErrno( EPERM );
				}
				
				target.StartTracing( current.GetPID() );
				
				return 0;
			}
			
			// Remaining requests require that the caller is tracing the target
			
			if ( target.GetTracingProcess() != current.GetPID() )
			{
				return frame.SetErrno( ESRCH );
			}
			
			if ( request == PTRACE_KILL )
			{
				target.DeliverSignal( SIGKILL );
				
				return 0;
			}
			
			// Remaining requests require tha the target be stoped
			
			if ( target.GetSchedule() != kProcessStopped )
			{
				return frame.SetErrno( ESRCH );
			}
			
			switch ( request )
			{
				case PTRACE_GETREGS:
					if ( addr == NULL )
					{
						return frame.SetErrno( EFAULT );
					}
					
					*(SavedRegisters*) addr = target.GetSavedRegisters();
					
					break;
				
				case PTRACE_CONT:
					if ( data > 0  &&  data < NSIG  &&  data != SIGSTOP )
					{
						target.DeliverSignal( data );
					}
					else if ( data == 0 )
					{
						target.Continue();
					}
					else
					{
						return frame.SetErrno( EIO );
					}
					
					break;
				
				case PTRACE_DETACH:
					target.StopTracing();
					
					break;
				
				default:
					return frame.SetErrno( EINVAL );
			}
		}
		catch ( ... )
		{
			return frame.SetErrnoFromException();
		}
		
		return 0;
	}
	
	REGISTER_SYSTEM_CALL( ptrace );
	
}

