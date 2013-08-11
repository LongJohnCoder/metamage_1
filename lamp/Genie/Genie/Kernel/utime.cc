/*	========
 *	utime.cc
 *	========
 */

// POSIX
#include "sys/stat.h"

// vfs
#include "vfs/primitives/touch.hh"
#include "vfs/primitives/utime.hh"

// Genie
#include "Genie/current_process.hh"
#include "Genie/FileDescriptors.hh"
#include "Genie/FS/ResolvePathAt.hh"
#include "Genie/FS/ResolvePathname.hh"
#include "Genie/SystemCallRegistry.hh"


#ifndef AT_SYMLINK_NOFOLLOW
#define AT_SYMLINK_NOFOLLOW  0
#endif


namespace Genie
{
	
	static inline bool merely_touch( const timespec* t )
	{
		if ( t != NULL )
		{
		#ifdef UTIME_NOW
			
			if ( t[0].tv_nsec == UTIME_NOW  &&  t[1].tv_nsec == UTIME_NOW )
			{
				return true;
			}
			
		#endif
		}
		
		return t == NULL;
	}
	
	static int utimensat( int fd, const char* path, const timespec times[2], int flags )
	{
		try
		{
			FSTreePtr file = path != NULL ? ResolvePathAt( fd, path )
			                              : get_filehandle( fd ).GetFile();
			
			const bool nofollow = flags & AT_SYMLINK_NOFOLLOW;
			
			if ( !nofollow  &&  path != NULL )
			{
				ResolveLinks_InPlace( file );
			}
			
			if ( merely_touch( times ) )
			{
				touch( file.get() );
			}
			else
			{
				utime( file.get(), times );
			}
		}
		catch ( ... )
		{
			return set_errno_from_exception();
		}
		
		return 0;
	}
	
	#pragma force_active on
	
	REGISTER_SYSTEM_CALL( utimensat );
	
	#pragma force_active reset
	
}

