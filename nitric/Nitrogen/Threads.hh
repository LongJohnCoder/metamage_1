// Nitrogen/Threads.hh
// -------------------
//
// Maintained by Joshua Juran

// Part of the Nitrogen project.
//
// Written 2004-2009 by Joshua Juran.
//
// This code was written entirely by the above contributor, who places it
// in the public domain.


#ifndef NITROGEN_THREADS_HH
#define NITROGEN_THREADS_HH

// Standard C/C++
#include <cstddef>

// Mac OS X
#ifdef __APPLE__
#include <CoreServices/CoreServices.h>
#endif

// Mac OS
#ifndef __THREADS__
#include <Threads.h>
#endif

// nucleus
#ifndef NUCLEUS_ENUMERATIONTRAITS_HH
#include "nucleus/enumeration_traits.hh"
#endif
#ifndef NUCLEUS_ERRORSREGISTERED_HH
#include "nucleus/errors_registered.hh"
#endif
#ifndef NUCLEUS_FLAGOPS_HH
#include "nucleus/flag_ops.hh"
#endif
#ifndef NUCLEUS_OBJECTPARAMETERTRAITS_HH
#include "nucleus/object_parameter_traits.hh"
#endif
#ifndef NUCLEUS_OWNED_HH
#include "nucleus/owned.hh"
#endif

// Nitrogen
#ifndef MAC_THREADS_TYPES_THREADID_HH
#include "Mac/Threads/Types/ThreadID.hh"
#endif
#ifndef MAC_TOOLBOX_UTILITIES_THROWOSSTATUS_HH
#include "Mac/Toolbox/Utilities/ThrowOSStatus.hh"
#endif

#ifndef NITROGEN_UPP_HH
#include "Nitrogen/UPP.hh"
#endif


namespace Nitrogen
{
	
	NUCLEUS_DECLARE_ERRORS_DEPENDENCY( ThreadManager );
	
	#pragma mark -
	#pragma mark ** Types and constants **
	
	enum ThreadState
	{
		kReadyThreadState   = ::kReadyThreadState,
		kStoppedThreadState = ::kStoppedThreadState,
		kRunningThreadState = ::kRunningThreadState,
		
		kThreadState_Max = nucleus::enumeration_traits< ::ThreadState >::max
	};
	
	using ::ThreadTaskRef;  // FIXME
	
	enum ThreadStyle
	{
		kCooperativeThread = ::kCooperativeThread,
		kPreemptiveThread  = ::kPreemptiveThread,
		
		kThreadStyle_Max = nucleus::enumeration_traits< ::ThreadStyle >::max
	};
	
	using Mac::ThreadID;
	
	using Mac::kNoThreadID;
	using Mac::kCurrentThreadID;
	using Mac::kApplicationThreadID;
	
	enum ThreadOptions
	{
		kNewSuspend       = ::kNewSuspend,
		kUsePremadeThread = ::kUsePremadeThread,
		kCreateIfNeeded   = ::kCreateIfNeeded,
		kFPUNotNeeded     = ::kFPUNotNeeded,
		kExactMatchThread = ::kExactMatchThread,
		
		kThreadOptions_Max = nucleus::enumeration_traits< ::ThreadOptions >::max
	};
	
	NUCLEUS_DEFINE_FLAG_OPS( ThreadOptions )
	
	// ...
	
  }

namespace Nitrogen
  {
	
	namespace Detail
	{
		
		class ThreadDisposer
		{
			private:
				bool recycleThread;
			
			public:
				typedef ThreadID  argument_type;
				typedef void      result_type;
				
				ThreadDisposer( bool recycleThread = false ) : recycleThread( recycleThread )  {}
				
				void operator()( ThreadID thread ) const
				{
					NUCLEUS_REQUIRE_ERRORS( ThreadManager );
					
					(void) ::DisposeThread( thread, NULL, recycleThread );
				}
		};
		
	}
	
	#pragma mark -
	#pragma mark ** ThreadEntryUPP **
	
	struct ThreadEntryUPP_Details : Basic_UPP_Details< ::ThreadEntryUPP,
	                                                   ::ThreadEntryProcPtr,
	                                                   ::NewThreadEntryUPP,
	                                                   ::DisposeThreadEntryUPP,
	                                                   ::InvokeThreadEntryUPP >
	{};
	
	typedef UPP< ThreadEntryUPP_Details > ThreadEntryUPP;
	
	typedef ThreadEntryUPP ThreadEntryTPP;
	
	inline nucleus::owned< ThreadEntryUPP > NewThreadEntryUPP( ::ThreadEntryProcPtr p )
	{
		return NewUPP< ThreadEntryUPP >( p );
	}
	
	inline void DisposeThreadEntryUPP( nucleus::owned< ThreadEntryUPP > )  {}
	
	inline void InvokeThreadEntryUPP( void*           threadParam,
	                                  ThreadEntryUPP  userUPP )
	{
		userUPP( threadParam );
	}
	
	template < class Param, class Result >
	struct ThreadEntry_Traits
	{
		typedef Result ( *ProcPtr )( Param param );
		
		static Result InvokeWithResult( ProcPtr proc, void* param )
		{
			return proc( nucleus::object_parameter_traits< Param  >::convert_from_pointer( param ) );
		}
	};
	
	template < class Result >
	struct ThreadEntry_Traits< void, Result >
	{
		typedef Result ( *ProcPtr )();
		
		static Result InvokeWithResult( ProcPtr proc, void* )
		{
			return proc();
		}
	};
	
	template < class Param, class Result >
	struct ThreadEntry_Invoke_Traits
	{
		typedef typename ThreadEntry_Traits< Param, Result >::ProcPtr ProcPtr;
		
		typedef ThreadEntry_Traits< Param, Result > Traits;
		
		static void* Invoke( ProcPtr proc, void* param )
		{
			Result result = Traits::InvokeWithResult( proc, param );
			
			return const_cast< void* >( reinterpret_cast< const void* >( result ) );
		}
	};
	
	template < class Param >
	struct ThreadEntry_Invoke_Traits< Param, void >
	{
		typedef typename ThreadEntry_Traits< Param, void >::ProcPtr ProcPtr;
		
		typedef ThreadEntry_Traits< Param, void > Traits;
		
		static void* Invoke( ProcPtr proc, void* param )
		{
			(void) Traits::InvokeWithResult( proc, param );
			
			return NULL;
		}
	};
	
#if TARGET_CPU_PPC && TARGET_RT_MAC_CFM
	
	void Terminate_ThreadStack();
	
#else
	
	inline void Terminate_ThreadStack()
	{
	}
	
#endif
	
	template < class Param,
	           class Result,
	           typename ThreadEntry_Traits< Param, Result >::ProcPtr threadEntry >
	struct ThreadEntry
	{
		static pascal void* Adapter( void* threadParam )
		{
			Terminate_ThreadStack();  // only affects PPC CFM
			
			try
			{
				return ThreadEntry_Invoke_Traits< Param, Result >::Invoke( threadEntry, threadParam );
			}
			catch ( ... )
			{
				return NULL;  // FIXME:  What if the thread routine throws?
			}
		}
	};
	
	#pragma mark -
	#pragma mark ** NewThread **
	
	// Level 0, returns nucleus::owned< ThreadID >
	
	nucleus::owned< ThreadID > NewThread( ThreadStyle     threadStyle,
	                                      ThreadEntryTPP  threadEntry,
	                                      void*           threadParam  = NULL,
	                                      std::size_t     stackSize    = std::size_t( 0 ),
	                                      ThreadOptions   options      = ThreadOptions(),
	                                      void**          threadResult = NULL );
	
	inline nucleus::owned< ThreadID > NewThread( ThreadEntryTPP  threadEntry,
	                                             void*           threadParam  = NULL,
	                                             std::size_t     stackSize    = std::size_t( 0 ),
	                                             ThreadOptions   options      = ThreadOptions(),
	                                             void**          threadResult = NULL )
	{
		return NewThread( kCooperativeThread,
		                  threadEntry,
		                  threadParam,
		                  stackSize,
		                  options,
		                  threadResult );
	}
	
	// Level 1, creates static UPP
	
	template < ::ThreadEntryProcPtr threadEntry >
	inline nucleus::owned< ThreadID > NewThread( ThreadStyle     threadStyle  = kCooperativeThread,
	                                             void*           threadParam  = NULL,
	                                             std::size_t     stackSize    = std::size_t( 0 ),
	                                             ThreadOptions   options      = ThreadOptions(),
	                                             void**          threadResult = NULL )
	{
		return NewThread( threadStyle,
		                  StaticUPP< ThreadEntryUPP, threadEntry >(),
		                  threadParam,
		                  stackSize,
		                  options,
		                  threadResult );
	}
	
	template < ::ThreadEntryProcPtr threadEntry >
	inline nucleus::owned< ThreadID > NewThread( void*           threadParam,
	                                             std::size_t     stackSize    = std::size_t( 0 ),
	                                             ThreadOptions   options      = ThreadOptions(),
	                                             void**          threadResult = NULL )
	{
		return NewThread< threadEntry >( kCooperativeThread,
		                                 threadParam,
		                                 stackSize,
		                                 options,
		                                 threadResult );
	}
	
	// Level 2, customizes param and result types
	
	template < class Param,
	           class Result,
	           typename ThreadEntry_Traits< Param, Result >::ProcPtr threadEntry >
	inline nucleus::owned< ThreadID >
	NewThread( ThreadStyle    threadStyle,
	           Param          param,
	           std::size_t    stackSize = std::size_t( 0 ),
	           ThreadOptions  options   = ThreadOptions(),
	           Result*        result    = NULL )
	{
		void*  threadParam  = const_cast< void* >( nucleus::object_parameter_traits< Param  >::convert_to_pointer( param ) );
		void** threadResult = reinterpret_cast< void** >( result );
		
		return NewThread< ThreadEntry< Param,
		                               Result,
		                               threadEntry >::Adapter >( threadStyle,
		                                                         threadParam,
		                                                         stackSize,
		                                                         options,
		                                                         threadResult );
	}
	
	template < class Param,
	           class Result,
	           typename ThreadEntry_Traits< Param, Result >::ProcPtr threadEntry >
	inline nucleus::owned< ThreadID >
	NewThread( Param          param,
	           std::size_t    stackSize = std::size_t( 0 ),
	           ThreadOptions  options   = ThreadOptions(),
	           Result*        result    = NULL )
	{
		return NewThread< ThreadEntry< Param,
		                               Result,
		                               threadEntry >::Adapter >( kCooperativeThread,
		                                                         param,
		                                                         stackSize,
		                                                         options,
		                                                         result );
	}
	
	// No result
	template < class Param,
	           typename ThreadEntry_Traits< Param, void >::ProcPtr threadEntry >
	inline nucleus::owned< ThreadID >
	NewThread( ThreadStyle    threadStyle,
	           Param          param,
	           std::size_t    stackSize = std::size_t( 0 ),
	           ThreadOptions  options   = ThreadOptions() )
	{
		void*  threadParam  = const_cast< void* >( nucleus::object_parameter_traits< Param  >::convert_to_pointer( param ) );
		
		return NewThread< ThreadEntry< Param,
		                               void,
		                               threadEntry >::Adapter >( threadStyle,
		                                                         threadParam,
		                                                         stackSize,
		                                                         options,
		                                                         NULL );
	}
	
	template < class Param,
	           typename ThreadEntry_Traits< Param, void >::ProcPtr threadEntry >
	inline nucleus::owned< ThreadID >
	NewThread( Param          param,
	           std::size_t    stackSize = std::size_t( 0 ),
	           ThreadOptions  options   = ThreadOptions() )
	{
		return NewThread< ThreadEntry< Param,
		                               void,
		                               threadEntry >::Adapter >( kCooperativeThread,
		                                                         param,
		                                                         stackSize,
		                                                         options );
	}
	
	// No param
	template < class Result,
	           typename ThreadEntry_Traits< void, Result >::ProcPtr threadEntry >
	inline nucleus::owned< ThreadID >
	NewThread( ThreadStyle    threadStyle = kCooperativeThread,
	           std::size_t    stackSize   = std::size_t( 0 ),
	           ThreadOptions  options     = ThreadOptions(),
	           Result*        result      = NULL )
	{
		void** threadResult = reinterpret_cast< void** >( result );
		
		return NewThread< ThreadEntry< void,
		                               Result,
		                               threadEntry >::Adapter >( threadStyle,
		                                                         NULL,
		                                                         stackSize,
		                                                         options,
		                                                         threadResult );
	}
	
	// No result or param
	template < typename ThreadEntry_Traits< void, void >::ProcPtr threadEntry >
	inline nucleus::owned< ThreadID >
	NewThread( ThreadStyle    threadStyle = kCooperativeThread,
	           std::size_t    stackSize   = std::size_t( 0 ),
	           ThreadOptions  options     = ThreadOptions() )
	{
		return NewThread< ThreadEntry< void,
		                               void,
		                               threadEntry >::Adapter >( threadStyle,
		                                                         NULL,
		                                                         stackSize,
		                                                         options,
		                                                         NULL );
	}
	
	
	void SetThreadScheduler( ThreadSchedulerTPP threadScheduler );
	
	void SetThreadSwitcher( ThreadID         thread,
	                        ThreadSwitchTPP  threadSwitcher,
	                        void*            switchProcParam,
	                        bool             inVsOut );
	
	void SetThreadTerminator( ThreadID              thread,
	                          ThreadTerminationTPP  threadTerminator,
	                          void *                terminationProcParam );
	
	void SetDebuggerNotificationProcs( DebuggerNewThreadTPP        notifyNewThread,
	                                   DebuggerDisposeThreadTPP    notifyDisposeThread,
	                                   DebuggerThreadSchedulerTPP  notifyThreadScheduler );
	
	void CreateThreadPool( ThreadStyle  threadStyle,
	                       std::size_t  numToCreate,
	                       std::size_t  stackSize );
	
	std::size_t GetFreeThreadCount( ThreadStyle threadStyle );
	
	std::size_t GetSpecificFreeThreadCount( ThreadStyle  threadStyle,
	                                        std::size_t  stackSize );
	
	std::size_t GetDefaultThreadStackSize( ThreadStyle threadStyle );
	
	inline std::size_t ThreadCurrentStackSpace( ThreadID thread )
	{
		UInt32 result;
		
		Mac::ThrowOSStatus( ::ThreadCurrentStackSpace( thread, &result ) );
		
		return result;
	}
	
	void DisposeThread( nucleus::owned< ThreadID >  thread,
	                    void*              threadResult,
	                    bool               recycleThread );
	
	void YieldToThread( ThreadID suggestedThread );
	
	void YieldToAnyThread();
	
	ThreadID MacGetCurrentThread();
	
	ThreadState GetThreadState( ThreadID thread );
	
	void SetThreadState( ThreadID     threadToSet,
	                     ThreadState  newState,
	                     ThreadID     suggestedThread = kNoThreadID );
	
	void SetThreadStateEndCritical( ThreadID     threadToSet,
	                                ThreadState  newState,
	                                ThreadID     suggestedThread = kNoThreadID );
	
	void ThreadBeginCritical();
	void ThreadEndCritical();
	
	ThreadTaskRef GetThreadCurrentTaskRef();
	
	ThreadState GetThreadStateGivenTaskRef( ThreadTaskRef  threadTaskRef,
	                                        ThreadID       threadToGet );
	
	void SetThreadReadyGivenTaskRef( ThreadTaskRef  threadTaskRef,
	                                 ThreadID       threadToSet );
	
}

#endif

