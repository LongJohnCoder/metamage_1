/*	====================
 *	FSTree_sys_window.cc
 *	====================
 */

#include "Genie/FileSystem/FSTree_sys_window.hh"

// POSeven
#include "POSeven/Errno.hh"

// Genie
#include "Genie/FileSystem/FSTree_sys_window_REF.hh"
#include "Genie/FileSystem/ResolvePathname.hh"


namespace Genie
{
	
	namespace NN = Nucleus;
	namespace p7 = poseven;
	
	
	typedef std::map< const FSTree*, boost::weak_ptr< const FSTree > > WindowMap;
	
	static WindowMap gWindowMap;
	
	
	FSTreePtr FSTree_sys_window::Lookup_Child( const std::string& name ) const
	{
		const FSTree* key = Pointer_KeyName_Traits< const FSTree* >::KeyFromName( name );
		
		WindowMap::const_iterator it = gWindowMap.find( key );
		
		if ( it == gWindowMap.end()  ||  it->second.expired() )
		{
			p7::throw_errno( ENOENT );
		}
		
		return it->second.lock();
	}
	
	void FSTree_sys_window::IterateIntoCache( FSTreeCache& cache ) const
	{
		WindowMap::const_iterator end = gWindowMap.end();
		
		for ( WindowMap::const_iterator it = gWindowMap.begin();  it != end;  ++it )
		{
			if ( it->second.expired() )
			{
				continue;
			}
			
			ino_t inode = (ino_t) it->first;  // coerce pointer to integer
			
			FSTreePtr window = it->second.lock();
			
			FSNode node( inode, window->Name() );
			
			cache.push_back( node );
		}
	}
	
	
	static void AddWindow( const FSTreePtr& member )
	{
		gWindowMap[ member.get() ] = member;
	}
	
	void RemoveWindow( const FSTree* window )
	{
		RemoveUserWindow( window );
		
		gWindowMap.erase( window );
	}
	
	static const FSTreePtr& SysWindow()
	{
		static FSTreePtr sys_window = ResolvePathname( "/sys/window" );
		
		return sys_window;
	}
	
	FSTreePtr NewWindow()
	{
		const FSTreePtr& parent = SysWindow();
		
		FSTreePtr window = Premapped_Factory< sys_window_REF_Mappings, &RemoveWindow >( parent, "/" );
		
		AddWindow( window );
		
		return window;
	}
	
}

