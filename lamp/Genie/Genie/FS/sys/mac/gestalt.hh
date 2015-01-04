/*
	Genie/FS/sys/mac/gestalt.hh
	---------------------------
*/

#ifndef GENIE_FS_SYS_MAC_GESTALT_HH
#define GENIE_FS_SYS_MAC_GESTALT_HH

// plus
#include "plus/string.hh"

// Genie
#include "Genie/FS/FSTreePtr.hh"


namespace Genie
{
	
	FSTreePtr New_FSTree_sys_mac_gestalt( const FSTree*        parent,
	                                      const plus::string&  name,
	                                      const void*          args );
	
}

#endif
