/*
	Genie/FS/sys/app.cc
	-------------------
*/

#include "Genie/FS/sys/app.hh"

// mac-sys-utils
#include "mac_sys/free_mem.hh"
#include "mac_sys/heap_size.hh"

// plus
#include "plus/serialize.hh"

// vfs
#include "vfs/node/types/fixed_dir.hh"
#include "vfs/node/types/property_file.hh"

// Genie
#include "Genie/FS/FSTree_fwd.hh"
#include "Genie/FS/property.hh"
#include "Genie/FS/sys/app/cmd.hh"
#include "Genie/FS/sys/app/dir.hh"
#include "Genie/FS/sys/app/exe.hh"
#include "Genie/FS/sys/app/menu.hh"
#include "Genie/FS/sys/app/window.hh"


namespace Genie
{
	
	struct GetFreeMem : plus::serialize_unsigned< long >
	{
		static long Get()
		{
			return mac::sys::free_mem();
		}
	};
	
	struct GetHeapSize : plus::serialize_unsigned< long >
	{
		static long Get()
		{
			return mac::sys::heap_size();
		}
	};
	
	template < class Accessor >
	struct sys_app_Property : readonly_property
	{
		static const int fixed_size = Accessor::fixed_size;
		
		typedef typename Accessor::result_type result_type;
		
		static void get( plus::var_string& result, const FSTree* that, bool binary )
		{
			const result_type data = Accessor::Get();
			
			Accessor::deconstruct::apply( result, data, binary );
		}
	};
	
	
	#define PREMAPPED( map )  &vfs::fixed_dir_factory, (const void*) map
	
	#define PROPERTY( prop )  &vfs::new_property, &property_params_factory< prop >::value
	
	#define PROPERTY_ACCESS( access )  PROPERTY( sys_app_Property< access > )
	
	const vfs::fixed_mapping sys_app_Mappings[] =
	{
		{ "cmd",   &New_FSTree_sys_app_cmd },
		
		{ "dir",   &New_FSTree_sys_app_dir },
		{ "exe",   &New_FSTree_sys_app_exe },
		
		{ "freemem", PROPERTY_ACCESS( GetFreeMem ) },
		
	#if !TARGET_API_MAC_CARBON
		
		{ "heapsize", PROPERTY_ACCESS( GetHeapSize ) },
		
	#endif
		
		{ "menu",   PREMAPPED( sys_app_menu_Mappings   ) },
		{ "window", PREMAPPED( sys_app_window_Mappings ) },
		
		{ NULL, NULL }
		
	};
	
}
