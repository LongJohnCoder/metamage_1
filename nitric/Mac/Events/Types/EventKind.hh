/*
	Mac/Events/Types/EventKind.hh
	-----------------------------
*/

#ifndef MAC_EVENTS_TYPES_EVENTKIND_HH
#define MAC_EVENTS_TYPES_EVENTKIND_HH

// Mac OS X
#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

// Mac OS
#ifndef __EVENTS__
#include <Events.h>
#endif

// nucleus
#ifndef NUCLEUS_ENUMERATIONTRAITS_HH
#include "nucleus/enumeration_traits.hh"
#endif


namespace Mac
{
	
	enum EventKind
	{
		nullEvent       = ::nullEvent,
		mouseDown       = ::mouseDown,
		mouseUp         = ::mouseUp,
		keyDown         = ::keyDown,
		keyUp           = ::keyUp,
		autoKey         = ::autoKey,
		updateEvt       = ::updateEvt,
		diskEvt         = ::diskEvt,
		activateEvt     = ::activateEvt,
		osEvt           = ::osEvt,
		kHighLevelEvent = ::kHighLevelEvent,
		
		kEventKind_Max = nucleus::enumeration_traits< ::EventKind >::max
	};
	
}

#endif
