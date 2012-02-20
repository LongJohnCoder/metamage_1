/*
	Genie/FS/sys/mac/proc.cc
	------------------------
*/

#include "Genie/FS/sys/mac/proc.hh"

// Standard C++
#include <algorithm>

// POSIX
#include <sys/stat.h>

// gear
#include "gear/hexidecimal.hh"

// Nitrogen
#include "Nitrogen/Processes.hh"

// poseven
#include "poseven/types/errno_t.hh"

// Genie
#include "Genie/FS/basic_directory.hh"
#include "Genie/FS/FSSpec.hh"
#include "Genie/FS/FSTreeCache.hh"
#include "Genie/FS/FSTree_Directory.hh"
#include "Genie/FS/FSTree_Property.hh"
#include "Genie/FS/link_method_set.hh"
#include "Genie/FS/node_method_set.hh"
#include "Genie/FS/utf8_text_property.hh"


namespace Genie
{
	
	namespace N = Nitrogen;
	namespace p7 = poseven;
	
	
	static plus::string encoded_ProcessSerialNumber( const ProcessSerialNumber& psn )
	{
		const bool extended = psn.highLongOfPSN != 0;
		
		plus::string name;
		
		const size_t eight = sizeof psn.highLongOfPSN * 2;
		
		const size_t more = extended ? eight + sizeof '-' : 0;
		
		char* p = name.reset( more + eight );
		
		if ( extended )
		{
			gear::encode_32_bit_hex( psn.highLongOfPSN, p );
			
			p[ eight ] = '-';
		}
		
		gear::encode_32_bit_hex( psn.lowLongOfPSN, &p[ more ] );
		
		return name;
	}
	
	
	static ProcessSerialNumber decoded_ProcessSerialNumber( const plus::string& name )
	{
		ProcessSerialNumber psn = { 0, 0 };
		
		const char* begin = name.c_str();
		const char* end   = name.c_str() + name.size();
		
		UInt32 high = 0;
		
		if ( const char* hyphen = std::strchr( begin, '-' ) )
		{
			if ( hyphen - begin != sizeof (UInt32) * 2 )
			{
				return psn;
			}
			
			high = gear::decode_32_bit_hex( begin );
			
			if ( high == 0 )
			{
				return psn;
			}
			
			begin = hyphen + 1;
		}
		
		if ( end - begin != sizeof (UInt32) * 2 )
		{
			return psn;
		}
		
		psn.highLongOfPSN = high;
		
		psn.lowLongOfPSN = gear::decode_32_bit_hex( begin );
		
		return psn;
	}
	
	
	static ProcessSerialNumber GetKeyFromParent( const FSTree* parent )
	{
		return decoded_ProcessSerialNumber( parent->Name() );
	}
	
	
	static bool is_valid_ProcessSerialNumber( const ProcessSerialNumber& key )
	{
		if ( key.lowLongOfPSN == 0  &&  key.highLongOfPSN == 0 )
		{
			return false;
		}
		
		try
		{
			(void) N::GetProcessInformation( key );
		}
		catch ( const Mac::OSStatus& err )
		{
			if ( err != procNotFound )
			{
				throw;
			}
			
			return false;
		}
		
		return true;
	}
	
	
	extern const FSTree_Premapped::Mapping sys_mac_proc_PSN_Mappings[];
	
	static FSTreePtr psn_lookup( const FSTreePtr& parent, const plus::string& name )
	{
		const ProcessSerialNumber psn = decoded_ProcessSerialNumber( name.c_str() );
		
		const bool canonical = encoded_ProcessSerialNumber( psn ) == name;
		
		if ( !canonical  ||  !is_valid_ProcessSerialNumber( psn ) )
		{
			poseven::throw_errno( ENOENT );
		}
		
		return Premapped_Factory( parent, name, sys_mac_proc_PSN_Mappings );
	}
	
	class psn_IteratorConverter
	{
		public:
			FSNode operator()( const ProcessSerialNumber& psn ) const
			{
				const ino_t inode = 0;
				
				plus::string name = encoded_ProcessSerialNumber( psn );
				
				return FSNode( inode, name );
			}
	};
	
	static void psn_iterate( const FSTreePtr& parent, FSTreeCache& cache )
	{
		psn_IteratorConverter converter;
		
		N::Process_Container sequence = N::Processes();
		
		std::transform( sequence.begin(),
		                sequence.end(),
		                std::back_inserter( cache ),
		                converter );
	}
	
	
	class sys_mac_proc_PSN_name : public readonly_property
	{
		private:
			typedef ProcessSerialNumber Key;
		
		public:
			static void get( plus::var_string& result, const FSTree* that, bool binary )
			{
				Key key = GetKeyFromParent( that );
				
				Str255 name;
				
				ProcessInfoRec processInfo;
				
				nucleus::initialize< ProcessInfoRec >( processInfo );
				
				processInfo.processName = name;
				
				N::GetProcessInformation( key, processInfo );
				
				result = name;
			}
	};
	
	static FSTreePtr mac_proc_exe_resolve( const FSTree* node )
	{
		ProcessSerialNumber psn = GetKeyFromParent( node->owner() );
		
		const FSSpec file = N::GetProcessAppSpec( psn );
		
		return FSTreeFromFSSpec( file, FileIsOnServer( file ) );
	}
	
	static const link_method_set mac_proc_exe_link_methods =
	{
		NULL,
		&mac_proc_exe_resolve
	};
	
	static const node_method_set mac_proc_exe_methods =
	{
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		&mac_proc_exe_link_methods
	};
	
	
	static FSTreePtr Executable_Factory( const FSTreePtr&     parent,
	                                     const plus::string&  name,
	                                     const void*          args )
	{
		return new FSTree( parent, name, S_IFLNK | 0777, &mac_proc_exe_methods );
	}
	
	#define PROPERTY( prop )  &new_property, &property_params_factory< prop >::value
	
	const FSTree_Premapped::Mapping sys_mac_proc_PSN_Mappings[] =
	{
		{ ".mac-name", PROPERTY( sys_mac_proc_PSN_name ) },
		
		{ "name", PROPERTY( utf8_text_property< sys_mac_proc_PSN_name > ) },
		
		{ "exe", &Executable_Factory },
		
		{ NULL, NULL }
	};
	
	FSTreePtr New_FSTree_sys_mac_proc( const FSTreePtr&     parent,
	                                   const plus::string&  name,
	                                   const void*          args )
	{
		return new_basic_directory( parent, name, psn_lookup, psn_iterate );
	}
	
}

