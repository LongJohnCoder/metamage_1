/*
	Genie/FS/sys/mac/vol/list/N/parms.cc
	------------------------------------
*/

#include "Genie/FS/sys/mac/vol/list/N/parms.hh"

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
#include "Mac/Toolbox/Utilities/ThrowOSStatus.hh"

// Genie
#include "Genie/FS/FSTree_Property.hh"


namespace Genie
{
	
	static Mac::FSVolumeRefNum GetKeyFromParent( const FSTreePtr& parent )
	{
		return Mac::FSVolumeRefNum( -gear::parse_unsigned_decimal( parent->Name().c_str() ) );
	}
	
	static Mac::FSVolumeRefNum GetKey( const FSTree* that )
	{
		return GetKeyFromParent( that->ParentRef() );
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
				throw FSTree_Property::Undefined();
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
				throw FSTree_Property::Undefined();
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
				throw FSTree_Property::Undefined();
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
				throw FSTree_Property::Undefined();
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
				throw FSTree_Property::Undefined();
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
				throw FSTree_Property::Undefined();
			}
			
			return (const char*) parmsInfo.vMDeviceID;
		}
	};
	
	static void GetVolParmsInfo( GetVolParmsInfoBuffer&  parmsInfo,
	                             const FSTree*           that )
	{
		const Mac::FSVolumeRefNum vRefNum = GetKey( that );
		
		memset( &parmsInfo, '\0', sizeof parmsInfo );
		
		HParamBlockRec pb = { 0 };
		
		HIOParam& io = pb.ioParam;
		
		io.ioVRefNum  = vRefNum;
		io.ioBuffer   = (char *) &parmsInfo;
		io.ioReqCount = sizeof parmsInfo;
		
		Mac::ThrowOSStatus( ::PBHGetVolParmsSync( &pb ) );
	}
	
	
	template < class Accessor >
	struct sys_mac_vol_N_Parms_Property : readonly_property
	{
		static const size_t fixed_size = Accessor::fixed_size;
		
		typedef Mac::FSVolumeRefNum Key;
		
		static void get( plus::var_string& result, const FSTree* that, bool binary )
		{
			GetVolParmsInfoBuffer parmsInfo;
			
			GetVolParmsInfo( parmsInfo, that );
			
			const typename Accessor::result_type data = Accessor::Get( parmsInfo );
			
			Accessor::deconstruct::apply( result, data, binary );
		}
	};
	
	
	#define PROPERTY( prop )  &new_property, &property_params_factory< sys_mac_vol_N_Parms_Property< prop > >::value
	
	const FSTree_Premapped::Mapping sys_mac_vol_N_parms_Mappings[] =
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

