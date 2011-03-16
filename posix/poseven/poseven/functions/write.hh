// write.hh
// --------
//
// Maintained by Joshua Juran

// Part of the Nitrogen project.
//
// Written 2008 by Joshua Juran.
//
// This code was written entirely by the above contributor, who places it
// in the public domain.


#ifndef POSEVEN_FUNCTIONS_WRITE_HH
#define POSEVEN_FUNCTIONS_WRITE_HH

// POSIX
#include <sys/types.h>

// iota
#ifndef IOTA_STRINGTRAITS_HH
#include "iota/string_traits.hh"
#endif

// poseven
#ifndef POSEVEN_TYPES_FD_T_HH
#include "poseven/types/fd_t.hh"
#endif


namespace poseven
{
	
	ssize_t write( fd_t fd, const char* buffer, size_t bytes_requested );
	
	template < class String >
	inline ssize_t write( fd_t fd, const String& string )
	{
		return write( fd,
		              iota::get_string_data( string ),
		              iota::get_string_size( string ) );
	}
	
}

#endif

