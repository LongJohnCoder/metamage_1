/*
	Genie/FS/sys/mac/gdev/list.cc
	-----------------------------
*/

#include "Genie/FS/sys/mac/gdev/list.hh"

// gear
#include "gear/inscribe_decimal.hh"

// plus
#include "plus/contains.hh"
#include "plus/hexadecimal.hh"
#include "plus/var_string.hh"
#include "plus/string/concat.hh"

// nucleus
#include "nucleus/linked_list_sequence.hh"

// poseven
#include "poseven/types/errno_t.hh"

// Nitrogen
#include "Nitrogen/Quickdraw.hh"

// MacFeatures
#include "MacFeatures/ColorQuickdraw.hh"

// vfs
#include "vfs/dir_contents.hh"
#include "vfs/dir_entry.hh"
#include "vfs/node.hh"
#include "vfs/node/types/fixed_dir.hh"
#include "vfs/node/types/symbolic_link.hh"

// Genie
#include "Genie/FS/basic_directory.hh"
#include "Genie/FS/FSTree_Property.hh"
#include "Genie/FS/property.hh"
#include "Genie/FS/serialize_qd.hh"
#include "Genie/Utilities/canonical_32_bit_hex.hh"


namespace Nitrogen
{
	
	class DeviceList_ContainerSpecifics
	{
		public:
			typedef GDHandle  value_type;
			typedef UInt32    size_type;
			typedef SInt32    difference_type;
			
			value_type get_next_value( value_type value )
			{
				return GetNextDevice( value );
			}
			
			static value_type begin_value()  { return GetDeviceList(); }
	};
	
	class DeviceList_Container: public nucleus::linked_list_sequence< ::Nitrogen::DeviceList_ContainerSpecifics >
	{
		friend DeviceList_Container DeviceList();
		
		private:
			DeviceList_Container()
			: nucleus::linked_list_sequence< ::Nitrogen::DeviceList_ContainerSpecifics >( ::Nitrogen::DeviceList_ContainerSpecifics() )
			{
			}
	};
	
	inline DeviceList_Container DeviceList()
	{
		return DeviceList_Container();
	}
	
}

namespace Genie
{
	
	namespace N = Nitrogen;
	namespace p7 = poseven;
	
	
	static GDHandle GetKeyFromParent( const FSTree* parent )
	{
		return (GDHandle) plus::decode_32_bit_hex( parent->name() );
	}
	
	
	static bool is_valid_GDHandle_name( const plus::string& name )
	{
		if ( !canonical_32_bit_hex::applies( name ) )
		{
			return false;
		}
		
		const GDHandle gdH = (GDHandle) plus::decode_32_bit_hex( name );
		
		return plus::contains( N::DeviceList(), gdH );
	}
	
	
	extern const vfs::fixed_mapping sys_mac_gdev_list_H_Mappings[];
	
	static FSTreePtr gdev_lookup( const FSTree* parent, const plus::string& name )
	{
		if ( !is_valid_GDHandle_name( name ) )
		{
			poseven::throw_errno( ENOENT );
		}
		
		return fixed_dir( parent, name, sys_mac_gdev_list_H_Mappings );
	}
	
	static void gdev_iterate( const FSTree* parent, vfs::dir_contents& cache )
	{
		N::DeviceList_Container sequence = N::DeviceList();
		
		typedef N::DeviceList_Container::const_iterator Iter;
		
		const Iter end = sequence.end();
		
		for ( Iter it = sequence.begin();  it != end;  ++it )
		{
			const ino_t inode = 0;
			
			plus::string name = plus::encode_32_bit_hex( (unsigned) *it );
			
			cache.push_back( vfs::dir_entry( inode, name ) );
		}
	}
	
	
	struct GetGDBounds : serialize_Rect
	{
		typedef const Rect& Result;
		
		static Result Get( GDHandle gdevice )
		{
			return gdevice[0]->gdRect;
		}
	};
	
	struct GetGDSize : serialize_Point
	{
		typedef Point Result;
		
		static Result Get( GDHandle gdevice )
		{
			const Rect& bounds = gdevice[0]->gdRect;
			
			const Point size = { bounds.bottom - bounds.top,
			                     bounds.right - bounds.left };
			
			return size;
		}
	};
	
	template < class Accessor >
	struct sys_mac_gdev_list_N_Property : readonly_property
	{
		static const int fixed_size = Accessor::fixed_size;
		
		typedef GDHandle Key;
		
		static void get( plus::var_string& result, const FSTree* that, bool binary )
		{
			Key key = GetKeyFromParent( that );
			
			const typename Accessor::Result data = Accessor::Get( key );
			
			Accessor::deconstruct::apply( result, data, binary );
		}
	};
	
	static FSTreePtr Driver_Link_Factory( const FSTree*        parent,
	                                      const plus::string&  name,
	                                      const void*          args )
	{
		GDHandle key = GetKeyFromParent( parent );
		
		plus::string unit = gear::inscribe_decimal( ~key[0]->gdRefNum );
		
		unit = "/sys/mac/unit/" + unit;
		
		return vfs::new_symbolic_link( parent, name, unit );
	}
	
	#define PROPERTY( prop )  &new_property, &property_params_factory< sys_mac_gdev_list_N_Property< prop > >::value
	
	const vfs::fixed_mapping sys_mac_gdev_list_H_Mappings[] =
	{
		{ "driver", &Driver_Link_Factory },
		
		{ "bounds", PROPERTY( GetGDBounds ) },
		
		{ NULL, NULL }
	};
	
	FSTreePtr New_FSTree_sys_mac_gdev_list( const FSTree*        parent,
	                                        const plus::string&  name,
	                                        const void*          args )
	{
		if ( !MacFeatures::Has_ColorQuickdraw() )
		{
			p7::throw_errno( ENOENT );
		}
		
		return new_basic_directory( parent, name, gdev_lookup, gdev_iterate );
	}
	
}
