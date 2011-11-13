/*
	v68k-macros.cc
	--------------
*/

// Standard C
#include <string.h>

// v68k
#include "v68k/macros.hh"

// tap-out
#include "tap/test.hh"


#pragma exceptions off


static const unsigned n_tests = 9;


using tap::ok_if;


static void sign_extend()
{
	using namespace v68k;
	
	ok_if( sign_extend( 0, byte_sized ) == 0 );
	ok_if( sign_extend( 0, word_sized ) == 0 );
	ok_if( sign_extend( 0, long_sized ) == 0 );
	
	ok_if( sign_extend( 0x12345678, byte_sized ) == 0x00000078 );
	ok_if( sign_extend( 0x12345678, word_sized ) == 0x00005678 );
	ok_if( sign_extend( 0x12345678, long_sized ) == 0x12345678 );
	
	ok_if( sign_extend( 0xFEDCBA98, byte_sized ) == 0xFFFFFF98 );
	ok_if( sign_extend( 0xFEDCBA98, word_sized ) == 0xFFFFBA98 );
	ok_if( sign_extend( 0xFEDCBA98, long_sized ) == 0xFEDCBA98 );
}

int main( int argc, char** argv )
{
	tap::start( "v68k-macros", n_tests );
	
	sign_extend();
	
	return 0;
}

