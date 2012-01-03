/*	========
 *	mkdir.cc
 *	========
 */

// POSIX
#include "sys/stat.h"

// Genie
#include "Genie/current_process.hh"
#include "Genie/FS/FSTree.hh"
#include "Genie/FS/ResolvePathAt.hh"
#include "Genie/SystemCallRegistry.hh"


namespace Genie
{
	
	static int mkdirat( int dirfd, const char* path, mode_t mode )
	{
		try
		{
			FSTreePtr location = ResolvePathAt( dirfd, path );
			
			// Do not resolve links
			
			location->CreateDirectory( mode );
		}
		catch ( ... )
		{
			return set_errno_from_exception();
		}
		
		return 0;
	}
	
	#pragma force_active on
	
	REGISTER_SYSTEM_CALL( mkdirat );
	
	#pragma force_active reset
	
}

