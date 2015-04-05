// perror.hh
// ---------

// Written 2009 by Joshua Juran.
//
// This code was written entirely by the above contributor, who places it
// in the public domain.


#ifndef POSEVEN_FUNCTIONS_PERROR_HH
#define POSEVEN_FUNCTIONS_PERROR_HH

// Standard C/C++
#include <cerrno>

// plus
#include "plus/string_fwd.hh"


namespace poseven
{
	
	void perror( int errnum = errno );
	
	void perror( const char* s, int errnum = errno );
	
	void perror( const char* s1, const char* s2, int errnum = errno );
	
	void perror( const char* s1, const char* s2, const char* s3 );
	
	void perror( const plus::string& s, int errnum = errno );
	
	void perror( const plus::string& s1, const plus::string& s2, int errnum = errno );
	
	void perror( const plus::string& s1, const plus::string& s2, const plus::string& s3 );
	
}

#endif
