/*
	Genie/FS/sys.cc
	---------------
*/

#include "Genie/FS/sys.hh"

// Standard C++
#include <algorithm>

// gear
#include "gear/hexidecimal.hh"

// Nitrogen
#include "Mac/Sound/Functions/SysBeep.hh"

// poseven
#include "poseven/types/errno_t.hh"

// Genie
#include "Genie/code/fixed_address.hh"
#include "Genie/FS/basic_directory.hh"
#include "Genie/FS/FSTreeCache.hh"
#include "Genie/FS/premapped.hh"
#include "Genie/FS/sys/app.hh"
#include "Genie/FS/sys/cpu.hh"
#include "Genie/FS/sys/mac.hh"
#include "Genie/FS/sys/type.hh"
#include "Genie/SystemCallRegistry.hh"


namespace Genie
{
	
	namespace p7 = poseven;
	
	
	typedef const SystemCall* SystemCallPtr;
	
	
	static plus::string name_of_syscall( SystemCallPtr key )
	{
		if ( key->name == NULL )
		{
			plus::string name;
			
			char* p = name.reset( sizeof '.' + sizeof (unsigned) * 2 );  // 9
			
			p[0] = '.';
			
			gear::encode_32_bit_hex( (unsigned) key, &p[1] );
			
			return name;
		}
		
		return key->name;
	}
	
	
	class FSTree_sys_kernel_bin_EXE : public FSTree
	{
		private:
			shared_exec_handle its_exec_handle;
		
		public:
			FSTree_sys_kernel_bin_EXE( const FSTreePtr&     parent,
			                           const plus::string&  name,
			                           lamp_entry           main )
			:
				FSTree( parent, name, S_IFREG | 0500 ),
				its_exec_handle( new fixed_address( main ) )
			{
			}
			
			shared_exec_handle GetExecutable() const
			{
				return its_exec_handle;
			}
	};
	
	
	static FSTreePtr syscall_lookup( const FSTreePtr& parent, const plus::string& name )
	{
		if ( LookUpSystemCallByName( name.c_str() ) == NULL )
		{
			poseven::throw_errno( ENOENT );
		}
		
		return seize_ptr( new FSTree_Premapped( parent, name ) );
	}
	
	class syscall_IteratorConverter
	{
		public:
			FSNode operator()( const SystemCall& value ) const
			{
				const ino_t inode = &value - GetSystemCall( 0 );
				
				plus::string name = name_of_syscall( &value );
				
				return FSNode( inode, name );
			}
	};
	
	static void syscall_iterate( const FSTreePtr& parent, FSTreeCache& cache )
	{
		syscall_IteratorConverter converter;
		
		std::transform( gSystemCallArray,
		                gSystemCallArray + gLastSystemCall,
		                std::back_inserter( cache ),
		                converter );
	}
	
	static FSTreePtr New_sys_kernel_syscall( const FSTreePtr&     parent,
	                                         const plus::string&  name,
	                                         const void*          args )
	{
		return new_basic_directory( parent, name, syscall_lookup, syscall_iterate );
	}
	
	
	namespace
	{
		
		int main_true()
		{
			return 0;
		}
		
		int main_false()
		{
			return 1;
		}
		
		int main_beep()
		{
			Mac::SysBeep();
			
			return 0;
		}
		
	}
	
	static FSTreePtr Executable_Factory( const FSTreePtr&     parent,
	                                     const plus::string&  name,
	                                     const void*          args )
	{
		return seize_ptr( new FSTree_sys_kernel_bin_EXE( parent, name, (lamp_entry) args ) );
	}
	
	#define EXEC( main )  &Executable_Factory, (const void*) &main
	
	extern const FSTree_Premapped::Mapping sys_kernel_bin_Mappings[];
	
	const FSTree_Premapped::Mapping sys_kernel_bin_Mappings[] =
	{
		{ "true",  EXEC( main_true  ) },
		{ "false", EXEC( main_false ) },
		
		{ "beep",  EXEC( main_beep  ) },
		
		{ NULL, NULL }
	};
	
	
	#define PREMAPPED( map )  &premapped_factory, (const void*) map
	
	extern const FSTree_Premapped::Mapping sys_kernel_Mappings[];
	
	const FSTree_Premapped::Mapping sys_kernel_Mappings[] =
	{
		{ "bin",     PREMAPPED( sys_kernel_bin_Mappings ) },
		
		{ "syscall", &New_sys_kernel_syscall },
		
		{ NULL, NULL }
	};
	
	const FSTree_Premapped::Mapping sys_Mappings[] =
	{
		{ "app",    PREMAPPED( sys_app_Mappings    ) },
		{ "cpu",    PREMAPPED( sys_cpu_Mappings    ) },
		{ "kernel", PREMAPPED( sys_kernel_Mappings ) },
		{ "mac",    PREMAPPED( sys_mac_Mappings    ) },
		{ "type",   PREMAPPED( sys_type_Mappings   ) },
		
		{ NULL, NULL }
	};
	
}

