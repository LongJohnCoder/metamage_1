/*
	Genie/FS/sys/mac/crm/serial.cc
	------------------------------
*/

#if defined( __MACOS__ )  &&  !TARGET_API_MAC_CARBON

#include "Genie/FS/sys/mac/crm/serial.hh"

// iota
#include "iota/decimal.hh"

// poseven
#include "poseven/types/errno_t.hh"

// ClassicToolbox
#include "ClassicToolbox/CRMSerialDevices.hh"

// Genie
#include "Genie/FS/basic_directory.hh"
#include "Genie/FS/FSTreeCache.hh"
#include "Genie/FS/FSTree_Directory.hh"
#include "Genie/FS/FSTree_Generated.hh"
#include "Genie/FS/FSTree_Property.hh"


namespace Genie
{
	
	namespace N = Nitrogen;
	
	
	static inline N::CRMDeviceID GetKeyFromParent( const FSTree* parent )
	{
		return N::CRMDeviceID( iota::parse_decimal( parent->Name().c_str() ) );
	}
	
	
	static CRMRecPtr GetCRMRecPtrFromID( N::CRMDeviceID id )
	{
		N::CRMResource_Container crmResources = N::CRMResources( N::crmSerialDevice );
		
		typedef N::CRMResource_Container::const_iterator Iter;
		
		for ( Iter it = crmResources.begin();  it != crmResources.end();  ++it )
		{
			CRMRecPtr rec = *it;
			
			if ( rec->crmDeviceID == id )
			{
				return rec;
			}
		}
		
		return NULL;
	}
	
	static CRMSerialPtr CRMRecGetSerial( CRMRecPtr crmRec )
	{
		// FIXME:  Throw if device type is wrong
		return N::GetCRMAttributes< N::crmSerialDevice >( crmRec );
	}
	
	extern const FSTree_Premapped::Mapping sys_mac_crm_serial_N_Mappings[];
	
	static FSTreePtr serial_lookup( const FSTreePtr& parent, const plus::string& name )
	{
		const N::CRMDeviceID key = N::CRMDeviceID( iota::parse_decimal( name.c_str() ) );
		
		// We can't just call N::CRMSearch< N::crmSerialDevice >( key ),
		// because that returns the *next* record.  So, linear search it is.
		
		if ( GetCRMRecPtrFromID( key ) == NULL )
		{
			poseven::throw_errno( ENOENT );
		}
		
		return Premapped_Factory( parent, name, sys_mac_crm_serial_N_Mappings );
	}
	
	class crm_IteratorConverter
	{
		public:
			FSNode operator()( const N::CRMSerialDevice_Container::value_type& rec ) const
			{
				const int key = rec->crmDeviceID;
				
				const ino_t inode = key;
				
				plus::string name = iota::inscribe_decimal( key );
				
				return FSNode( inode, name );
			}
	};
	
	static void serial_iterate( const FSTreePtr& parent, FSTreeCache& cache )
	{
		crm_IteratorConverter converter;
		
		N::CRMSerialDevice_Container sequence = N::CRMSerialDevices();
		
		std::transform( sequence.begin(),
		                sequence.end(),
		                std::back_inserter( cache ),
		                converter );
	}
	
	
	typedef StringHandle CRMSerialRecord::*StringSelector;
	
	static plus::string GetSelectedString( const FSTree* that, StringSelector selector )
	{
		N::CRMDeviceID key = GetKeyFromParent( that );
		
		CRMRecPtr crmRec = GetCRMRecPtrFromID( key );
		
		CRMSerialPtr serialPtr = CRMRecGetSerial( crmRec );
		
		StringHandle h = serialPtr->*selector;
		
		return plus::string( N::Str255( *h ) );
	}
	
	// These are necessary because CW Pro 6 doesn't support pointer-to-member template parameters.
	
	struct sys_mac_crm_serial_N_name : readonly_property
	{
		static void get( plus::var_string& result, const FSTree* that, bool binary )
		{
			result = GetSelectedString( that, &CRMSerialRecord::name );
		}
	};
	
	struct sys_mac_crm_serial_N_input : readonly_property
	{
		static void get( plus::var_string& result, const FSTree* that, bool binary )
		{
			result = GetSelectedString( that, &CRMSerialRecord::inputDriverName );
		}
	};
	
	struct sys_mac_crm_serial_N_output : readonly_property
	{
		static void get( plus::var_string& result, const FSTree* that, bool binary )
		{
			result = GetSelectedString( that, &CRMSerialRecord::outputDriverName );
		}
	};
	
	
	struct sys_mac_crm_serial_N_icon
	{
		static plus::string Read( const FSTree* parent, const plus::string& name )
		{
			N::CRMDeviceID key = GetKeyFromParent( parent );
			
			CRMRecPtr crmRec = GetCRMRecPtrFromID( key );
			
			CRMSerialPtr serialPtr = CRMRecGetSerial( crmRec );
			
			CRMIconHandle iconHandle = serialPtr->deviceIcon;
			
			CRMIconRecord icon = **iconHandle;
			
			// 32-bit * 32-bit = 1024 bits = 128 bytes, x2 = 256 bytes
			const std::size_t maskedIconSize = sizeof icon.oldIcon + sizeof icon.oldMask;
			
			plus::string iconData( (const char*) &icon.oldIcon, maskedIconSize );
			
			return iconData;
		}
	};
	
	
	#define PROPERTY( prop )  &new_property, &property_params_factory< prop >::value
	
	const FSTree_Premapped::Mapping sys_mac_crm_serial_N_Mappings[] =
	{
		{ "name",   PROPERTY( sys_mac_crm_serial_N_name   ) },
		{ "input",  PROPERTY( sys_mac_crm_serial_N_input  ) },
		{ "output", PROPERTY( sys_mac_crm_serial_N_output ) },
		
		{ "icon",   &new_generated, (void*) &sys_mac_crm_serial_N_icon::Read },
		
		{ NULL, NULL }
	};
	
	FSTreePtr New_FSTree_sys_mac_crm_serial( const FSTreePtr&     parent,
	                                         const plus::string&  name,
	                                         const void*          args )
	{
		return new_basic_directory( parent, name, serial_lookup, serial_iterate );
	}
	
}

#endif

