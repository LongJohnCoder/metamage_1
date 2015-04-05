// link.hh
// -------
//
// Maintained by Joshua Juran

// Part of the Nitrogen project.
//
// Written 2009 by Joshua Juran.
//
// This code was written entirely by the above contributor, who places it
// in the public domain.


#ifndef POSEVEN_FUNCTIONS_LINK_HH
#define POSEVEN_FUNCTIONS_LINK_HH

// iota
#include "iota/string_traits.hh"


namespace poseven
{
	
	void link( const char* from, const char* to );
	
	template < class String1, class String2 >
	inline void link( const String1& from, const String2& to )
	{
		using iota::get_string_c_str;
		
		link( get_string_c_str( from ), get_string_c_str( to ) );
	}
	
}

#endif
