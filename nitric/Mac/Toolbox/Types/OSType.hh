/*
	Mac/Toolbox/Types/OSType.hh
	---------------------------
*/

#ifndef MAC_TOOLBOX_TYPES_OSTYPE_HH
#define MAC_TOOLBOX_TYPES_OSTYPE_HH

// Mac OS X
#ifdef __APPLE__
#include <CoreServices/CoreServices.h>
#endif

// Mac OS
#ifndef __MACTYPES__
#include <MacTypes.h>
#endif

// nucleus
#ifndef NUCLEUS_ENUMERATIONTRAITS_HH
#include "nucleus/enumeration_traits.hh"
#endif


namespace Mac
{
	
	enum OSType
	{
		kUnknownType = ::kUnknownType,
		
		kOSType_Max = nucleus::enumeration_traits< ::OSType >::max
	};
	
}

#endif

