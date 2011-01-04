// Nitrogen/Timer.hh
// -----------------
//
// Maintained by Joshua Juran

// Part of the Nitrogen project.
//
// Written 2004-2009 by Joshua Juran.
//
// This code was written entirely by the above contributor, who places it
// in the public domain.


#ifndef NITROGEN_TIMER_HH
#define NITROGEN_TIMER_HH

// Mac OS
#ifndef __TIMER__
#include <Timer.h>
#endif

// nucleus
#ifndef NUCLEUS_ERRORSREGISTERED_HH
#include "nucleus/errors_registered.hh"
#endif

// Nitrogen
#ifndef MAC_TIMER_FUNCTIONS_MICROSECONDS_HH
#include "Mac/Timer/Functions/Microseconds.hh"
#endif

#ifndef NITROGEN_OSSTATUS_HH
#include "Nitrogen/OSStatus.hh"
#endif
#ifndef NITROGEN_UPP_HH
#include "Nitrogen/UPP.hh"
#endif


namespace Nitrogen
{
	
	NUCLEUS_DECLARE_ERRORS_DEPENDENCY( TimeManager );
	
}

namespace nucleus
{
	
	template <>
	struct disposer< TMTaskPtr >
	{
		typedef TMTaskPtr  argument_type;
		typedef void       result_type;
		
		void operator()( TMTaskPtr tmTaskPtr ) const
		{
			NUCLEUS_REQUIRE_ERRORS( Nitrogen::TimeManager );
			
			const QElemPtr qElem = (QElemPtr) tmTaskPtr;
			
			(void) ::RemoveTimeTask( qElem );
		}
	};
	
}

namespace Nitrogen
{
	
	typedef pascal void (*TimerProcPtr)( TMTaskPtr tmTaskPtr );
	
#if TARGET_CPU_68K && !TARGET_RT_MAC_CFM
	
	struct TimerUPP_Details
	{
		typedef ::TimerUPP UPPType;
		
		// This is the stack-based function signature
		typedef TimerProcPtr ProcPtr;
		
		template < ProcPtr procPtr >
		static pascal void Glue()
		{
			Call_With_A0_Glue< ProcPtr, procPtr >();
		}
	};
	
	typedef GlueUPP< TimerUPP_Details > TimerUPP;
	
#else
	
	struct TimerUPP_Details : Basic_UPP_Details< ::TimerUPP,
	                                             ::TimerProcPtr,
	                                             ::NewTimerUPP,
	                                             ::DisposeTimerUPP,
	                                             ::InvokeTimerUPP >
	{
	};
	
	typedef UPP< TimerUPP_Details > TimerUPP;
	
#endif
	
	nucleus::owned< TMTaskPtr > InstallTimeTask ( TMTask& tmTask );
	nucleus::owned< TMTaskPtr > InstallXTimeTask( TMTask& tmTask );
	
	void PrimeTimeTask( TMTask& tmTask, long count );
	
	void RemoveTimeTask( nucleus::owned< TMTaskPtr > tmTask );
	
	// ...
	
	using Mac::Microseconds;
	
	inline nucleus::owned< TimerUPP > NewTimerUPP( TimerProcPtr p )
	{
		return NewUPP< TimerUPP >( p );
	}
	
	inline void DisposeTimerUPP( nucleus::owned< TimerUPP > )
	{
	}
	
	inline void InvokeTimerUPP( TMTaskPtr  tmTaskPtr,
	                            TimerUPP   userUPP )
	{
		::InvokeTimerUPP( tmTaskPtr, userUPP.Get() );
	}
	
}

#endif

