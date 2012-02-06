/*
	Genie/FS/app.cc
	---------------
*/

#include "Genie/FS/app.hh"

// Standard C++
#include <map>

// poseven
#include "poseven/types/errno_t.hh"

// Genie
#include "Genie/FS/basic_directory.hh"
#include "Genie/FS/file-tests.hh"
#include "Genie/FS/FSTreeCache.hh"
#include "Genie/FS/node_method_set.hh"
#include "Genie/FS/ResolvePathname.hh"
#include "Genie/FS/SymbolicLink.hh"


namespace Genie
{
	
	namespace p7 = poseven;
	
	
	typedef std::map< plus::string, plus::string > app_map;
	
	app_map the_application_map;
	
	
	static void unused_app_slot_symlink( const FSTree*        node,
	                                     const plus::string&  target )
	{
		if ( target.c_str()[0] != '/' )
		{
			p7::throw_errno( EINVAL );
		}
		
		plus::string& value = the_application_map[ node->Name() ];
		
		if ( !value.empty() )
		{
			p7::throw_errno( EEXIST );
		}
		
		value = target;
	}
	
	static node_method_set unused_app_slot_methods =
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
		&unused_app_slot_symlink
	};
	
	
	class app_symlink : public FSTree_SymbolicLink
	{
		public:
			app_symlink( const FSTreePtr&     parent,
			             const plus::string&  name,
			             const plus::string&  target )
			:
				FSTree_SymbolicLink( parent, name, target )
			{
			}
			
			void Delete() const;
	};
	
	void app_symlink::Delete() const
	{
		app_map::iterator it = the_application_map.find( Name() );
		
		if ( it == the_application_map.end() )
		{
			p7::throw_errno( ENOENT );
		}
		
		the_application_map.erase( it );
	}
	
	static bool check_existence( app_map::iterator it )
	{
		const plus::string& target = it->second;
		
		try
		{
			FSTreePtr port = ResolveAbsolutePath( target );
			
			if ( exists( port ) )
			{
				return true;
			}
		}
		catch ( ... )
		{
		}
		
		the_application_map.erase( it );
		
		return false;
	}
	
	static FSTreePtr app_lookup( const FSTreePtr& parent, const plus::string& name )
	{
		app_map::iterator it = the_application_map.find( name );
		
		if ( it != the_application_map.end()  &&  check_existence( it ) )
		{
			const plus::string& target = it->second;
			
			return new app_symlink( parent, name, target );
		}
		
		return new FSTree( parent, name, 0, &unused_app_slot_methods );
	}
	
	static void app_iterate( const FSTreePtr& parent, FSTreeCache& cache )
	{
		typedef app_map::iterator iterator;
		
		const iterator end = the_application_map.end();
		
		for ( iterator next = the_application_map.begin();  next != end;  )
		{
			iterator it = next++;
			
			if ( check_existence( it ) )
			{
				const ino_t inode = 0;
				
				const plus::string& name = it->first;
				
				cache.push_back( FSNode( inode, name ) );
			}
		}
	}
	
	
	FSTreePtr New_FSTree_app( const FSTreePtr&     parent,
	                          const plus::string&  name,
	                          const void*          args )
	{
		return new_basic_directory( parent, name, app_lookup, app_iterate );
	}
	
}

