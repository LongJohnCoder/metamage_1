// Nitrogen/Processes.hh
// ---------------------
//
// Maintained by Joshua Juran

// Part of the Nitrogen project.
//
// Written 2004-2009 by Joshua Juran.
//
// This code was written entirely by the above contributor, who places it
// in the public domain.


#ifndef NITROGEN_PROCESSES_HH
#define NITROGEN_PROCESSES_HH

// Mac OS X
#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif

// Mac OS
#ifndef __PROCESSES__
#include <Processes.h>
#endif

// nucleus
#ifndef NUCLEUS_ADVANCEUNTILERRORSEQUENCE_HH
#include "nucleus/advance_until_error_sequence.hh"
#endif
#ifndef NUCLEUS_ENUMERATIONTRAITS_HH
#include "nucleus/enumeration_traits.hh"
#endif
#ifndef NUCLEUS_ERRORSREGISTERED_HH
#include "nucleus/errors_registered.hh"
#endif
#ifndef NUCLEUS_FLAGOPS_HH
#include "nucleus/flag_ops.hh"
#endif
#ifndef NUCLEUS_INITIALIZE_HH
#include "nucleus/initialize.hh"
#endif
#ifndef NUCLEUS_MAKE_HH
#include "nucleus/make.hh"
#endif

#ifndef NITROGEN_OSSTATUS_HH
#include "Nitrogen/OSStatus.hh"
#endif


namespace Nitrogen
{
	
	NUCLEUS_DECLARE_ERRORS_DEPENDENCY( ProcessManager );
	
	enum LaunchFlags
	{
		launchContinue    = ::launchContinue,
		launchNoFileFlags = ::launchNoFileFlags,
		
		kLaunchFlags_Max = nucleus::enumeration_traits< ::LaunchFlags >::max
	};
	
	NUCLEUS_DEFINE_FLAG_OPS( LaunchFlags )
	
	template < unsigned long > struct LowLongOfPSN  {};
	
}

namespace nucleus
{
	
	template <>
	struct maker< ProcessSerialNumber >
	{
		ProcessSerialNumber operator()( unsigned long high, unsigned long low ) const
		{
			ProcessSerialNumber result;
			result.highLongOfPSN = high;
			result.lowLongOfPSN  = low;
			return result;
		}

		template < unsigned long k >
		ProcessSerialNumber operator()( Nitrogen::LowLongOfPSN< k > ) const
		{
			::ProcessSerialNumber result = { 0, k };
			return result;
		}
	};
}

inline bool operator==( const ::ProcessSerialNumber& a, const ::ProcessSerialNumber& b )
{
	return a.highLongOfPSN == b.highLongOfPSN
		&& a.lowLongOfPSN  == b.lowLongOfPSN;
}

inline bool operator!=( const ::ProcessSerialNumber& a, const ::ProcessSerialNumber& b )
{
	return !( a == b );
}

namespace Nitrogen
{
	
	inline ProcessSerialNumber NoProcess()       { return nucleus::make< ProcessSerialNumber >( LowLongOfPSN< kNoProcess      >() ); }
	inline ProcessSerialNumber SystemProcess()   { return nucleus::make< ProcessSerialNumber >( LowLongOfPSN< kSystemProcess  >() ); }
	inline ProcessSerialNumber CurrentProcess()  { return nucleus::make< ProcessSerialNumber >( LowLongOfPSN< kCurrentProcess >() ); }
	
}

namespace nucleus
{
	
	template <>
	struct initializer< ProcessInfoRec >
	{
		ProcessInfoRec& operator()( ProcessInfoRec& processInfo, FSSpec* appSpec = NULL ) const
		{
			processInfo.processInfoLength = sizeof processInfo;
			processInfo.processName       = NULL;
			processInfo.processAppSpec    = appSpec;
			
			return processInfo;
		}
	};
	
	template <>
	struct maker< ProcessInfoRec >
	{
		ProcessInfoRec operator()( FSSpec* appSpec = NULL ) const
		{
			ProcessInfoRec result = { 0 };
			
			return nucleus::initialize( result, appSpec );
		}
	};
	
}

namespace Nitrogen
{
	
	// Nitrogen accessors, since no Carbon accessors exist
	inline FSSpec GetProcessInfoAppSpec( const ProcessInfoRec& processInfo )
	{
		return *processInfo.processAppSpec;
	}
	
	ProcessSerialNumber GetCurrentProcess();
	
	ProcessSerialNumber GetFrontProcess();
	
	bool SameProcess( const ProcessSerialNumber& a, 
		              const ProcessSerialNumber& b );
	
	void SetFrontProcess( const ProcessSerialNumber& psn );
	
	ProcessSerialNumber LaunchApplication( const FSSpec&   file,
	                                       LaunchFlags     launchFlags   = LaunchFlags(),
	                                       AppParameters*  appParameters = NULL );
	
	ProcessSerialNumber GetNextProcess( ProcessSerialNumber process );
	
	ProcessInfoRec& GetProcessInformation( const ProcessSerialNumber& process, ProcessInfoRec& info);
	ProcessInfoRec  GetProcessInformation( const ProcessSerialNumber& process );
	
	// 425
	FSRef GetProcessBundleLocation( const ProcessSerialNumber& psn );
	
	FSSpec GetProcessAppSpec( const ProcessSerialNumber& psn );
	
	class Process_ContainerSpecifics : public OSStatus_EndOfEnumeration< procNotFound >
	{
		public:
			typedef ProcessSerialNumber value_type;
			typedef UInt32 size_type;
			typedef SInt32 difference_type;
			typedef value_type key_type;
			
			static key_type get_next_key( const key_type& value )
			{
				return GetNextProcess( value );
			}
			
			static const key_type* get_pointer( const key_type& value )  { return &value; }
			
			static key_type begin_key()  { return get_next_key( NoProcess() ); }
			static key_type end_key()    { return               NoProcess()  ; }
	};
	
	class Process_Container: public nucleus::advance_until_error_sequence< ::Nitrogen::Process_ContainerSpecifics >
	{
		friend Process_Container Processes();
		
		private:
			Process_Container()
			: nucleus::advance_until_error_sequence< ::Nitrogen::Process_ContainerSpecifics >( ::Nitrogen::Process_ContainerSpecifics() )
			{}
	};
	
	inline Process_Container Processes()
	{
		return Process_Container();
	}
	
}

// Necessary for operators of types declared at global scope, such as ProcessSerialNumber.
#include "nucleus/operators.hh"

#endif

