// Nitrogen/Gestalt.cc
// -------------------
//
// Maintained by Joshua Juran

// Part of the Nitrogen project.
//
// Written 2002-2008 by Lisa Lippincott and Joshua Juran.
//
// This code was written entirely by the above contributors, who place it
// in the public domain.


#include "Nitrogen/Gestalt.hh"

// Mac OS
#ifndef __MACERRORS__
#include <MacErrors.h>
#endif

// Nitrogen
#include "Nitrogen/OSStatus.hh"


namespace Nitrogen
{
	
	// does nothing, but guarantees construction of theRegistration
	NUCLEUS_DEFINE_ERRORS_DEPENDENCY( GestaltManager )
	
	
	static void RegisterGestaltManagerErrors();
	
	
#if NUCLEUS_RICH_ERRORCODES
#pragma force_active on
	
	class GestaltManagerErrorsRegistration
	{
		public:
			GestaltManagerErrorsRegistration()  { RegisterGestaltManagerErrors(); }
	};
	
	static GestaltManagerErrorsRegistration theRegistration;
	
#pragma force_active reset
#endif
	
	SInt32 Gestalt( Gestalt_Selector selector )
	{
		SInt32 result;
		
		Mac::ThrowOSStatus( ::Gestalt( selector, &result ) );
		
		return result;
	}
	
	SInt32 Gestalt( Gestalt_Selector selector, SInt32 defaultValue )
	{
		SInt32 result;
		
		OSErr err = ::Gestalt( selector, &result );
		
		if ( err == gestaltUndefSelectorErr )
		{
			return defaultValue;
		}
		
		Mac::ThrowOSStatus( err );
		
		return result;
	}
	
	void RegisterGestaltManagerErrors()
	{
		RegisterOSStatus< memFullErr              >();
		RegisterOSStatus< envNotPresent           >();
		RegisterOSStatus< envBadVers              >();
		RegisterOSStatus< envVersTooBig           >();
		RegisterOSStatus< gestaltUnknownErr       >();
		RegisterOSStatus< gestaltUndefSelectorErr >();
		RegisterOSStatus< gestaltDupSelectorErr   >();
		RegisterOSStatus< gestaltLocationErr      >();
	}
	
}
