/*	=========
 *	unlink.cc
 *	=========
 */

// Standard C
#include <errno.h>

// POSIX
#include "fcntl.h"

// Genie
#include "Genie/FS/ResolvePathAt.hh"
#include "Genie/SystemCallRegistry.hh"
#include "Genie/SystemCalls.hh"


namespace Genie
{
	
	static int unlinkat( int dirfd, const char* path, int flags )
	{
		SystemCallFrame frame( "unlinkat" );
		
		try
		{
			FSTreePtr file = ResolvePathAt( dirfd, path );
			
			// Do not resolve links -- delete the symlink
			
			const bool remove_any = flags & AT_REMOVEANY;
			const bool remove_dir = flags & AT_REMOVEDIR;
			
			if ( remove_any || remove_dir == file->IsDirectory() )
			{
				file->Delete();
			}
			else
			{
				return frame.SetErrno( remove_dir ? ENOTDIR : EISDIR );
			}
		}
		catch ( ... )
		{
			return frame.SetErrnoFromException();
		}
		
		return 0;
	}
	
	#pragma force_active on
	
	REGISTER_SYSTEM_CALL( unlinkat );
	
	#pragma force_active reset
	
}

