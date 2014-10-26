/*
	test.hh
	
	Copyright 2009, Joshua Juran
*/

#ifndef TAPOUT_TEST_HH
#define TAPOUT_TEST_HH

// Standard C/C++
#include <cstring>

// iota
#include "iota/strings.hh"


#define STR_( x ) #x
#define STR( x ) STR_( x )

#define LINEREF()  __FILE__ ":" STR(__LINE__) ": test failed:\n"

#define EXPECT( cond )  ::tap::expect( cond, STR_LEN( LINEREF() ) )

#define EXPECT_NULL( p )  EXPECT( (p) == NULL )

#define EXPECT_CMP( p, p_len, q, q_len )  EXPECT( ::tap::cmp( (p), (p_len), (q), (q_len) ) == 0 )


namespace tap
{
	
	void start( const char* program_name, unsigned tests_planned );
	
	
	void print( bool ok );
	
	void expect( bool condition, const char* ref, unsigned len );
	
	int cmp( const void* p, unsigned p_len, const void* q, unsigned q_len );
	
}

#endif
