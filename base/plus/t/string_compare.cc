/*
	t/string_compare.cc
	-------------------
*/

// Standard C++
#include <stdexcept>

// Standard C
#include <string.h>

// iota
#include "iota/strings.hh"

// plus
#include "plus/string.hh"

// tap-out
#include "tap/test.hh"


static const unsigned n_tests = 24;


static void compare()
{
	plus::string test = "test";
	
	EXPECT( test.compare( test ) == 0 );
	
	EXPECT( test.compare( ""      ) >  0 );
	EXPECT( test.compare( "t"     ) >  0 );
	EXPECT( test.compare( "taste" ) >  0 );
	EXPECT( test.compare( "test"  ) == 0 );
	EXPECT( test.compare( "testy" ) <  0 );
	EXPECT( test.compare( "toast" ) <  0 );
	
	EXPECT( test.compare( 0, 0, test ) <  0 );
	EXPECT( test.compare( 0, 3, test ) <  0 );
	EXPECT( test.compare( 0, 4, test ) == 0 );
	EXPECT( test.compare( 0, 5, test ) == 0 );
	
	plus::string st = "st";
	
	EXPECT( test.compare( 0, 4, st ) >  0 );
	EXPECT( test.compare( 1, 4, st ) <  0 );
	EXPECT( test.compare( 2, 4, st ) == 0 );
	
	plus::string past = "past";
	
	EXPECT( test.compare( 0, 4, past, 0, 4 ) >  0 );
	EXPECT( test.compare( 0, 4, past, 1, 4 ) >  0 );
	EXPECT( test.compare( 1, 4, past, 0, 4 ) <  0 );
	EXPECT( test.compare( 1, 4, past, 1, 4 ) >  0 );
	EXPECT( test.compare( 2, 4, past, 2, 4 ) == 0 );
	
	EXPECT( test.compare( 0, 4, "testing", 4 ) == 0 );
	
	bool exception_thrown = false;
	
	try
	{
		test.compare( 5, 0, test );
	}
	catch ( const std::out_of_range& )
	{
		exception_thrown = true;
	}
	
	EXPECT( exception_thrown );
	
	exception_thrown = false;
	
	try
	{
		test.compare( 5, 0, test, 0, 4 );
	}
	catch ( const std::out_of_range& )
	{
		exception_thrown = true;
	}
	
	EXPECT( exception_thrown );
	
	exception_thrown = false;
	
	try
	{
		test.compare( 0, 4, test, 5, 0 );
	}
	catch ( const std::out_of_range& )
	{
		exception_thrown = true;
	}
	
	EXPECT( exception_thrown );
	
	exception_thrown = false;
	
	try
	{
		test.compare( 5, 0, test.c_str(), 4 );
	}
	catch ( const std::out_of_range& )
	{
		exception_thrown = true;
	}
	
	EXPECT( exception_thrown );
}

int main( int argc, const char *const *argv )
{
	tap::start( "string_compare", n_tests );
	
	compare();
	
	return 0;
}
