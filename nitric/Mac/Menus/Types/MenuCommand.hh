/*
	Mac/Menus/Types/MenuCommand.hh
	------------------------------
*/

#ifndef MAC_MENUS_TYPES_MENUCOMMAND_HH
#define MAC_MENUS_TYPES_MENUCOMMAND_HH

// Mac OS X
#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

// Mac OS
#ifndef __MENUS__
#include <Menus.h>
#endif

// nucleus
#ifndef NUCLEUS_ENUMERATIONTRAITS_HH
#include "nucleus/enumeration_traits.hh"
#endif


namespace Mac
{
	
	enum MenuCommand
	{
		kMenuCommand_Max = nucleus::enumeration_traits< ::MenuCommand >::max
	};
	
}

#endif

