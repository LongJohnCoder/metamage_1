// lseek.hh
// --------
//
// Maintained by Joshua Juran

// Part of the Nitrogen project.
//
// Written 2008 by Joshua Juran.
//
// This code was written entirely by the above contributor, who places it
// in the public domain.


#ifndef POSEVEN_FUNCTIONS_LSEEK_HH
#define POSEVEN_FUNCTIONS_LSEEK_HH

// POSIX
#include <unistd.h>

// poseven
#ifndef POSEVEN_TYPES_FD_T_HH
#include "poseven/types/fd_t.hh"
#endif


namespace poseven
{
	
	enum seek_t
	{
		seek_set = SEEK_SET,
		seek_cur = SEEK_CUR,
		seek_end = SEEK_END,
		
		seek_t_max = nucleus::enumeration_traits< int >::max
	};
	
	off_t lseek( fd_t fd, off_t offset, seek_t whence = seek_set );
	
	inline off_t lseek( fd_t fd )
	{
		return lseek( fd, 0, seek_cur );
	}
	
}

#endif
