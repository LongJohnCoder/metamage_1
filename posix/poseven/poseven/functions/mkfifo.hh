// mkfifo.hh
// ---------
//
// Maintained by Joshua Juran

// Part of the Nitrogen project.
//
// Written 2007 by Joshua Juran.
//
// This code was written entirely by the above contributor, who places it
// in the public domain.


#ifndef POSEVEN_FUNCTIONS_MKFIFO_HH
#define POSEVEN_FUNCTIONS_MKFIFO_HH

// iota
#include "iota/string_traits.hh"

// poseven
#ifndef POSEVEN_TYPES_MODE_T_HH
#include "poseven/types/mode_t.hh"
#endif


namespace poseven
{
	
	void mkfifo( const char* pathname, mode_t mode );
	
	template < class String >
	inline void mkfifo( const String& path, mode_t mode )
	{
		using iota::get_string_c_str;
		
		mkfifo( get_string_c_str( path ), mode );
	}
	
}

#endif

