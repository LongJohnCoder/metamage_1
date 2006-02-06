/*	========
 *	mkdir.cc
 *	========
 */

// Universal Interfaces
#include <MacTypes.h>

// Standard C
#include "errno.h"

// Standard C/C++
#include <cstdio>
#include <cstring>

// POSIX
#include "sys/stat.h"

// Nitrogen
#include "Nitrogen/OSStatus.h"

// Orion
#include "Orion/Main.hh"
#include "SystemCalls.hh"


namespace N = Nitrogen;
namespace O = Orion;


int O::Main( int argc, const char *const argv[] )
{
	// Check for sufficient number of args
	if ( argc < 2 )
	{
		std::fprintf( stderr, "mkdir: missing arguments\n" );
		return 1;
	}
	
	// Try to make each directory.  Return whether any errors occurred.
	int fail = 0;
	
	for ( int index = 1;  index < argc;  ++index )
	{
		int result = mkdir( argv[ index ], 0700 );
		
		if ( result == -1 )
		{
			std::fprintf( stderr,
			              "mkdir: %s: %s\n",
			                      argv[ index ],
			                          std::strerror( errno ) );
			fail++;
		}
	}
	
	return (fail == 0) ? 0 : 1;
}

