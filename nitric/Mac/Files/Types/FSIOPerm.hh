/*
	Mac/Files/Types/FSIOPerm.hh
	---------------------------
*/

#ifndef MAC_FILES_TYPES_FSIOPERM_HH
#define MAC_FILES_TYPES_FSIOPERM_HH

// Mac OS X
#ifdef __APPLE__
#include <CoreServices/CoreServices.h>
#endif

// Mac OS
#ifndef __FILES__
#include <Files.h>
#endif

// nucleus
#ifndef NUCLEUS_ENUMERATIONTRAITS_HH
#include "nucleus/enumeration_traits.hh"
#endif
#ifndef NUCLEUS_FLAGOPS_HH
#include "nucleus/flag_ops.hh"
#endif


namespace Mac
{
	
	enum FSIOPerm
	{
		fsCurPerm    = ::fsCurPerm,
		fsRdPerm     = ::fsRdPerm,
		fsWrPerm     = ::fsWrPerm,
		fsRdWrPerm   = ::fsRdWrPerm,
		fsRdWrShPerm = ::fsRdWrShPerm,
		fsRdDenyPerm = ::fsRdDenyPerm,
		fsWrDenyPerm = ::fsWrDenyPerm,
		
		kFSIOPerm_Max = nucleus::enumeration_traits< SInt8 >::max
	};
	
	NUCLEUS_DEFINE_FLAG_OPS( FSIOPerm )
	
	typedef FSIOPerm FSIOPermssn, FSIOPermissions;
	
}

#endif
