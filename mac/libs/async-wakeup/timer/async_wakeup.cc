/*
	async_wakeup.cc
	---------------
*/

#include "mac_sys/async_wakeup.hh"

// Mac OS X
#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif

// Mac OS
#ifndef MAC_OS_X_VERSION_10_8
#ifndef __OPENTRANSPORT__
#include <OpenTransport.h>
#endif
#endif
#ifndef __PROCESSES__
#include <Processes.h>
#endif
#ifndef __TIMER__
#include <Timer.h>
#endif

// mac-sys-utils
#include "mac_sys/current_process.hh"


#if TARGET_CPU_68K
	short CPUFlag : 0x12F;
	
	#define IN( reg ) : __##reg
#else
	#define IN( reg )  /**/
#endif

namespace mac {
namespace sys {
	
	static volatile bool wakeup_requested;
	
	struct wakeup_TMTask
	{
		TMTask                      tm;
		const ProcessSerialNumber*  psn;
	};
	
	static pascal void async_wakeup_proc( TMTaskPtr task IN( a1 ) )
	{
		if ( TARGET_API_MAC_CARBON  &&  ! wakeup_requested )
		{
			::RmvTime( (QElemPtr) task );
			
			task->qLink = 0;  // Release the lock.
			
			return;
		}
		
		::WakeUpProcess( ((const wakeup_TMTask*) task)->psn );
		
		::PrimeTime( (QElemPtr) task, 10 );  // 10ms
	}
	
	static wakeup_TMTask the_wakeup_task =
	{
		{
			0,
			0,
			::NewTimerUPP( (TimerProcPtr) &async_wakeup_proc ),
			0,
			0,
			0
		},
		&current_process()
	};
	
	static bool lock_timer()
	{
		register void** mutex = (void**) &the_wakeup_task.tm.qLink;
		
	#if TARGET_CPU_68K
		
		const short cpu68000 = 0;
		
		if ( CPUFlag == cpu68000 )
		{
			// 68000 only
			
			register bool acquired = false;  // Initialize to squelch warning
			
			asm
			{
				TAS    (mutex)
				SEQ    acquired
				NEG.B  acquired
			}
			
			return acquired;
		}
		
	#endif
		
	#ifndef MAC_OS_X_VERSION_10_8
		
		return ::OTCompareAndSwapPtr( NULL, (void*) 0xFFFFFFFF, mutex );
		
	#endif
		
		return false;
	}
	
	void request_async_wakeup()
	{
		wakeup_requested = true;
		
		if ( lock_timer() )
		{
			the_wakeup_task.tm.tmWakeUp   = 0;
			the_wakeup_task.tm.tmReserved = 0;
			
			::InsTime( (QElemPtr) &the_wakeup_task );
			
			::PrimeTime( (QElemPtr) &the_wakeup_task, 10 );  // 10ms
		}
		
		::WakeUpProcess( the_wakeup_task.psn );
	}
	
	void clear_async_wakeup()
	{
		if ( ! TARGET_API_MAC_CARBON  &&  wakeup_requested )
		{
			::RmvTime( (QElemPtr) &the_wakeup_task );
			
			the_wakeup_task.tm.qLink = 0;  // release the lock
		}
		
		wakeup_requested = false;
	}
	
}
}
