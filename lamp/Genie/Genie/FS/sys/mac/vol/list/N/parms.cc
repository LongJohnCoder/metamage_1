/*
	Genie/FS/sys/mac/vol/list/N/parms.cc
	------------------------------------
*/

#include "Genie/FS/sys/mac/vol/list/N/parms.hh"

// mac-types
#include "mac_types/GetVolParmsInfoBuffer.hh"

// mac-sys-utils
#include "mac_sys/volume_params.hh"

// gear
#include "gear/parse_decimal.hh"

// plus
#include "plus/deconstruct.hh"
#include "plus/freeze.hh"
#include "plus/serialize.hh"
#include "plus/stringify.hh"
#include "plus/var_string.hh"

// Nitrogen
#include "Mac/Files/Types/FSVolumeRefNum.hh"

// Genie
#include "Genie/FS/FSTree.hh"
#include "Genie/FS/FSTree_Property.hh"
#include "Genie/FS/property.hh"


namespace Genie
{
	
	using mac::types::GetVolParmsInfoBuffer;
	
	
	static Mac::FSVolumeRefNum GetKeyFromParent( const FSTree* parent )
	{
		return Mac::FSVolumeRefNum( -gear::parse_unsigned_decimal( parent->name().c_str() ) );
	}
	
	static Mac::FSVolumeRefNum GetKey( const FSTree* that )
	{
		return GetKeyFromParent( that->owner() );
	}
	
	
	struct GetVolumeParmsAttrib : plus::serialize_hex< UInt32 >
	{
		static UInt32 Get( const GetVolParmsInfoBuffer& parmsInfo )
		{
			return parmsInfo.vMAttrib;
		}
	};
	
	struct GetVolumeParmsHandle : plus::serialize_pointer
	{
		static ::Handle Get( const GetVolParmsInfoBuffer& parmsInfo )
		{
			if ( parmsInfo.vMLocalHand == 0 )
			{
				throw undefined_property();
			}
			
			return parmsInfo.vMLocalHand;
		}
	};
	
	struct GetVolumeParmsServer : plus::serialize_hex< UInt32 >
	{
		static UInt32 Get( const GetVolParmsInfoBuffer& parmsInfo )
		{
			if ( parmsInfo.vMServerAdr == 0 )
			{
				throw undefined_property();
			}
			
			return parmsInfo.vMServerAdr;
		}
	};
	
	struct GetVolumeParmsGrade : plus::serialize_int< SInt32 >
	{
		static SInt32 Get( const GetVolParmsInfoBuffer& parmsInfo )
		{
			if ( parmsInfo.vMVersion < 2  ||  parmsInfo.vMVolumeGrade == 0 )
			{
				throw undefined_property();
			}
			
			return parmsInfo.vMVolumeGrade;
		}
	};
	
	struct GetVolumeParmsPrivID : plus::serialize_int< SInt16 >
	{
		static SInt16 Get( const GetVolParmsInfoBuffer& parmsInfo )
		{
			if ( parmsInfo.vMVersion < 2 )
			{
				throw undefined_property();
			}
			
			return parmsInfo.vMForeignPrivID;
		}
	};
	
	struct GetVolumeParmsExtended : plus::serialize_hex< UInt32 >
	{
		static UInt32 Get( const GetVolParmsInfoBuffer& parmsInfo )
		{
			if ( parmsInfo.vMVersion < 3 )
			{
				throw undefined_property();
			}
			
			return parmsInfo.vMExtendedAttributes;
		}
	};
	
	struct GetVolumeParmsDeviceID : plus::serialize_c_string_contents
	{
		static const char* Get( const GetVolParmsInfoBuffer& parmsInfo )
		{
			if ( parmsInfo.vMVersion < 4  ||  parmsInfo.vMDeviceID == NULL )
			{
				throw undefined_property();
			}
			
			return (const char*) parmsInfo.vMDeviceID;
		}
	};
	
	
	template < class Accessor >
	struct sys_mac_vol_N_Parms_Property : readonly_property
	{
		static const int fixed_size = Accessor::fixed_size;
		
		typedef Mac::FSVolumeRefNum Key;
		
		static void get( plus::var_string& result, const FSTree* that, bool binary )
		{
			GetVolParmsInfoBuffer parmsInfo;
			
			mac::sys::get_volume_params( parmsInfo, GetKey( that ) );
			
			const typename Accessor::result_type data = Accessor::Get( parmsInfo );
			
			Accessor::deconstruct::apply( result, data, binary );
		}
	};
	
	
	#define PROPERTY( prop )  &new_property, &property_params_factory< sys_mac_vol_N_Parms_Property< prop > >::value
	
	const vfs::fixed_mapping sys_mac_vol_N_parms_Mappings[] =
	{
		{ "attrib",   PROPERTY( GetVolumeParmsAttrib   ) },
		{ "handle",   PROPERTY( GetVolumeParmsHandle   ) },
		{ "server",   PROPERTY( GetVolumeParmsServer   ) },
		
		{ "grade",    PROPERTY( GetVolumeParmsGrade    ) },
		{ "priv",     PROPERTY( GetVolumeParmsPrivID   ) },
		
		{ "extended", PROPERTY( GetVolumeParmsExtended ) },
		
		{ "device",   PROPERTY( GetVolumeParmsDeviceID ) },
		
		{ NULL, NULL }
		
	};
	
}

