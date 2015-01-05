/*
	ioctl.cc
	--------
*/

#include "relix/syscall/ioctl.hh"

// POSIX
#include <sys/ioctl.h>

// vfs
#include "vfs/file_descriptor.hh"
#include "vfs/filehandle/primitives/ioctl.hh"

// relix-kernel
#include "relix/api/get_fds.hh"
#include "relix/api/errno.hh"
#include "relix/task/fd_map.hh"


namespace relix
{
	
	int ioctl( int fd, unsigned long request, int* argp )
	{
		fd_map& files = get_fds();
		
		try
		{
			vfs::file_descriptor& file = files.at( fd );
			
			if ( request == FIOCLEX )
			{
				file.set_to_close_on_exec( true );
			}
			else if ( request == FIONCLEX )
			{
				file.set_to_close_on_exec( false );
			}
			else
			{
				ioctl( *file.handle, request, argp );
			}
		}
		catch ( ... )
		{
			return set_errno_from_exception();
		}
		
		return 0;
	}
	
}
