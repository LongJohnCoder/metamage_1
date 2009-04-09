/*	=================
 *	FSTree_sys_mac.cc
 *	=================
 */

#include "Genie/FS/FSTree_sys_mac.hh"

// Genie
#include "Genie/FS/FSTree_Generated.hh"
#include "Genie/FS/FSTree_Property.hh"
#include "Genie/FS/FSTree_sys_mac_desktop.hh"
#include "Genie/FS/FSTree_sys_mac_errata.hh"
#include "Genie/FS/FSTree_sys_mac_event.hh"
#include "Genie/FS/FSTree_sys_mac_gdev.hh"
#include "Genie/FS/FSTree_sys_mac_gestalt.hh"
#include "Genie/FS/FSTree_sys_mac_keys.hh"
#include "Genie/FS/FSTree_sys_mac_machine.hh"
#include "Genie/FS/FSTree_sys_mac_name.hh"
#include "Genie/FS/FSTree_sys_mac_proc.hh"
#include "Genie/FS/FSTree_sys_mac_rom.hh"
#include "Genie/FS/FSTree_sys_mac_thng.hh"
#if !TARGET_API_MAC_CARBON
#include "Genie/FS/FSTree_sys_mac_adb.hh"
#include "Genie/FS/FSTree_sys_mac_crm.hh"
#include "Genie/FS/FSTree_sys_mac_crsr.hh"
#include "Genie/FS/FSTree_sys_mac_drive.hh"
#include "Genie/FS/FSTree_sys_mac_unit.hh"
#include "Genie/FS/FSTree_sys_mac_xpram.hh"
#endif
#include "Genie/FS/FSTree_sys_mac_user.hh"
#include "Genie/FS/FSTree_sys_mac_vol.hh"
//#include "Genie/FS/FSTree_sys_mac_window.hh"


namespace Genie
{
	
	template < class Property >
	static FSTreePtr Property_Factory( const FSTreePtr&    parent,
	                                   const std::string&  name )
	{
		return New_FSTree_Property( parent,
		                            name,
		                            &Property::Read );
	}
	
	template < class Property >
	static FSTreePtr Generated_Factory( const FSTreePtr&    parent,
	                                    const std::string&  name )
	{
		return New_FSTree_Generated( parent,
		                             name,
		                             &Property::Read );
	}
	
	const FSTree_Premapped::Mapping sys_mac_Mappings[] =
	{
		{ "vol",    &New_FSTree_sys_mac_vol  },
		{ "gdev",   &New_FSTree_sys_mac_gdev },
		{ "proc",   &New_FSTree_sys_mac_proc },
		{ "thng",   &New_FSTree_sys_mac_thng },
		//{ "window", &Basic_Factory< FSTree_sys_mac_window > },
		
	#if !TARGET_API_MAC_CARBON
		
		{ "crm",  &Premapped_Factory< sys_mac_crm_Mappings  > },
		{ "crsr", &Premapped_Factory< sys_mac_crsr_Mappings > },
		
		{ "adb",   &New_FSTree_sys_mac_adb   },
		{ "drive", &New_FSTree_sys_mac_drive },
		{ "unit",  &New_FSTree_sys_mac_unit  },
		
		{ "xpram", &Generated_Factory< sys_mac_xpram > },
		
	#endif
		
		{ "desktop", &Premapped_Factory< sys_mac_desktop_Mappings > },
		{ "errata",  &Premapped_Factory< sys_mac_errata_Mappings  > },
		{ "event",   &Premapped_Factory< sys_mac_event_Mappings   > },
		{ "machine", &Premapped_Factory< sys_mac_machine_Mappings > },
		{ "user",    &Premapped_Factory< sys_mac_user_Mappings    > },
		
		{ "gestalt", &New_FSTree_sys_mac_gestalt },
		
		{ "name", &Property_Factory< sys_mac_name > },
		{ "keys", &Generated_Factory< sys_mac_keys > },
		
		{ "rom", &New_FSTree_sys_mac_rom },
		
		{ NULL, NULL }
		
	};
	
}

