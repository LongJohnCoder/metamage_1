/*
	Mac/Controls/Types/ControlProcID.hh
	-----------------------------------
*/

#ifndef MAC_CONTROLS_TYPES_CONTROLPROCID_HH
#define MAC_CONTROLS_TYPES_CONTROLPROCID_HH

// Mac OS X
#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

// Mac OS
#ifndef __CONTROLDEFINITIONS__
#include <ControlDefinitions.h>
#endif
#ifndef __CONTROLS__
#include <Controls.h>
#endif

// nucleus
#ifndef NUCLEUS_ENUMERATIONTRAITS_HH
#include "nucleus/enumeration_traits.hh"
#endif


namespace Mac
{
	
	enum ControlProcID
	{
		pushButProc               = ::pushButProc,
		checkBoxProc              = ::checkBoxProc,
		radioButProc              = ::radioButProc,
		scrollBarProc             = ::scrollBarProc,
		popupMenuProc             = ::popupMenuProc,
		kControlScrollBarProc     = ::kControlScrollBarProc,
		kControlScrollBarLiveProc = ::kControlScrollBarLiveProc,
		
		kControlProcID_Max = nucleus::enumeration_traits< SInt16 >::max
	};
	
}

#endif

