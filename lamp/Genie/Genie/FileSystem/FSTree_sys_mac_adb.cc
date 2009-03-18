/*	=====================
 *	FSTree_sys_mac_adb.cc
 *	=====================
 */

#if !TARGET_API_MAC_CARBON

#include "Genie/FileSystem/FSTree_sys_mac_adb.hh"

// ClassicToolbox
#include "ClassicToolbox/DeskBus.h"

// BitsAndBytes
#include "HexStrings.hh"

// Arcana
#include "ADBProtocol.hh"

// Genie
#include "Genie/FileSystem/FSTree_Directory.hh"
#include "Genie/FileSystem/FSTree_Generated.hh"
#include "Genie/FileSystem/FSTree_Property.hh"


namespace Genie
{
	
	namespace N = Nitrogen;
	
	
	struct ADBAddress_KeyName_Traits : Integer_KeyName_Traits< Nitrogen::ADBAddress >
	{
	};
	
	struct sys_mac_adb_Details : public ADBAddress_KeyName_Traits
	{
		typedef Nitrogen::ADBDevice_Container Sequence;
		
		static Sequence ItemSequence()  { return Nitrogen::ADBDevice_Container(); }
		
		static Key KeyFromValue( Sequence::const_reference ref )  { return ref; }
		
		static bool KeyIsValid( const Key& key );
		
		static FSTreePtr GetChildNode( const FSTreePtr&    parent,
		                               const std::string&  name,
		                               const Key&          key );
	};
	
	typedef FSTree_Sequence< sys_mac_adb_Details > FSTree_sys_mac_adb;
	
	
	static inline N::ADBAddress GetKeyFromParent( const FSTreePtr& parent )
	{
		return ADBAddress_KeyName_Traits::KeyFromName( parent->Name() );
	}
	
	static N::ADBAddress GetKey( const FSTree* that )
	{
		return GetKeyFromParent( that->ParentRef() );
	}
	
	
	bool sys_mac_adb_Details::KeyIsValid( const Key& key )
	{
		try
		{
			return N::GetADBInfo( key ).devType != 0;
		}
		catch ( ... )
		{
			return false;
		}
	}
	
	
	extern const FSTree_Premapped::Mapping sys_mac_adb_N_Mappings[];
	
	FSTreePtr sys_mac_adb_Details::GetChildNode( const FSTreePtr&    parent,
		                                         const std::string&  name,
		                                         const Key&          key )
	{
		return Premapped_Factory< sys_mac_adb_N_Mappings >( parent, name );
	}
	
	
	class sys_mac_adb_N_type
	{
		private:
			typedef N::ADBAddress Key;
		
		public:
			static std::string Read( const FSTree* that, bool binary )
			{
				Key key = GetKey( that );
				
				using BitsAndBytes::ByteAsHex;
				
				ADBDataBlock data = N::GetADBInfo( key );
				
				return ByteAsHex( data.devType );
			}
	};
	
	class sys_mac_adb_N_origin
	{
		private:
			typedef N::ADBAddress Key;
		
		public:
			static std::string Read( const FSTree* that, bool binary )
			{
				Key key = GetKey( that );
				
				ADBDataBlock data = N::GetADBInfo( key );
				
				char c = '0' + data.origADBAddr;
				
				return std::string( c, 1 );
			}
	};
	
	static void WriteADBRegister( N::ADBAddress address, int i, std::string& result )
	{
		using BitsAndBytes::ByteAsHex;
		
		ADBRegister reg = GetADBRegister( address, i );
		
		for ( unsigned i = 1;  i <= reg.buffer[0];  ++i )
		{
			result += ByteAsHex( reg.buffer[i] );
			
			result += ' ';
		}
		
		if ( reg.buffer[0] != 0 )
		{
			result.resize( result.size() - 1 );  // kill extra space
		}
		
		result += '\n';
	}
	
	class sys_mac_adb_N_registers
	{
		private:
			typedef N::ADBAddress Key;
		
		public:
			static std::string Read( const FSTree* that )
			{
				Key key = GetKey( that );
				
				std::string output;
				
				WriteADBRegister( key, 0, output );
				WriteADBRegister( key, 1, output );
				WriteADBRegister( key, 2, output );
				WriteADBRegister( key, 3, output );
				
				return output;
			}
	};
	
	
	template < class Property >
	static FSTreePtr Property_Factory( const FSTreePtr&    parent,
	                                   const std::string&  name )
	{
		return New_FSTree_Property( parent,
		                            name,
		                            &Property::Read );
	}
	
	static FSTreePtr Registers_Factory( const FSTreePtr&    parent,
	                                    const std::string&  name )
	{
		return New_FSTree_Generated( parent,
		                             name,
		                             &sys_mac_adb_N_registers::Read );
	}
	
	const FSTree_Premapped::Mapping sys_mac_adb_N_Mappings[] =
	{
		{ "type",      &Property_Factory< sys_mac_adb_N_type   > },
		{ "origin",    &Property_Factory< sys_mac_adb_N_origin > },
		{ "registers", &Registers_Factory                        },
		
		{ NULL, NULL }
	};
	
	FSTreePtr New_FSTree_sys_mac_adb( const FSTreePtr& parent, const std::string& name )
	{
		return FSTreePtr( new FSTree_sys_mac_adb( parent, name ) );
	}
	
}

#endif

