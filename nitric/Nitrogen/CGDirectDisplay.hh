// Nitrogen/CGDirectDisplay.hh
// ---------------------------
//
// Maintained by Joshua Juran

// Part of the Nitrogen project.
//
// Written 2004-2006 by Joshua Juran and Marshall Clow.
//
// This code was written entirely by the above contributors, who place it
// in the public domain.


#ifndef NITROGEN_CGDIRECTDISPLAY_HH
#define NITROGEN_CGDIRECTDISPLAY_HH

// Mac OS X
#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif

// Mac OS
#ifndef CGDIRECTDISPLAY_H_
#ifndef __CGDIRECT_DISPLAY_H__
#ifndef __CGDIRECTDISPLAY__
#include <CGDirectDisplay.h>
#endif
#endif
#endif
	
// Nitrogen
//	we need this include in order to instantiate Owned<CGrafPtr>
#ifndef NITROGEN_QUICKDRAW_HH
#include "Nitrogen/Quickdraw.hh"
#endif


namespace nucleus
  {
	template <> struct disposer< CGDirectDisplayID >
	{
		typedef CGDirectDisplayID  argument_type;
		typedef void               result_type;
		
		void operator()( CGDirectDisplayID display ) const
		{
			::CGDisplayRelease( display );
		}
	};
  }

namespace Nitrogen
  {
	
	// This belongs in Nitrogen/Quickdraw.hh, but it depends on CGDirectDisplayID,
	// which is defined in CGDirectDisplay.h which isn't included by Quickdraw.h,
	// so we move it here so as not to create a new header dependency.
	// (It's defined in Nitrogen/Quickdraw.cc.)
	nucleus::owned< CGrafPtr > CreateNewPortForCGDisplayID( CGDirectDisplayID display );
	
	nucleus::owned< CGDirectDisplayID > CGDisplayCapture( CGDirectDisplayID display = 0 );
	
	inline void CGDisplayRelease( nucleus::owned< CGDirectDisplayID > )  {}
	
}

#endif
