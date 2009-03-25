/*	================
 *	ResolvePathAt.cc
 *	================
 */

#include "Genie/FileSystem/ResolvePathAt.hh"

// POSIX
#include "fcntl.h"

// Genie
#include "Genie/FileDescriptors.hh"
#include "Genie/FileSystem/ResolvePathname.hh"
#include "Genie/IO/Directory.hh"
#include "Genie/Process.hh"


namespace Genie
{
	
	static FSTreePtr GetDirFile( int fd )
	{
		return GetFileHandleWithCast< DirHandle >( fd ).GetFile();
	}
	
	FSTreePtr ResolvePathAt( int dirfd, const std::string& path )
	{
		const bool absolute = !path.empty() && path[0] == '/';
		
		return ResolvePathname( path,   absolute          ? FSTreePtr()
		                              : dirfd == AT_FDCWD ? CurrentProcess().GetCWD()
			                          :                     GetDirFile( dirfd ) );
	}
	
}

