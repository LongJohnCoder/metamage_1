/*	======
 *	cat.cc
 *	======
 */

// Standard C/C++
#include <cerrno>
#include <cstdio>
#include <cstring>

// POSIX
#include <fcntl.h>
#include <unistd.h>

// POSeven
#include "POSeven/Errno.hh"
#include "POSeven/FileDescriptor.hh"
#include "POSeven/IOPump.hh"
#include "POSeven/Open.hh"

// Orion
#include "Orion/Main.hh"


namespace NN = Nucleus;
namespace p7 = poseven;
namespace O = Orion;


static bool PathnameMeansStdIn( const char* pathname )
{
	return    pathname[0] == '-'
	       && pathname[1] == '\0';
}

static const char* EvaluateMetaFilename( const char* pathname )
{
	if ( PathnameMeansStdIn( pathname ) )
	{
		return "/dev/fd/0";
	}
	
	return pathname;
}

int O::Main( int argc, argv_t argv )
{
	iota::arg_t argv0 = argv[0];
	
	iota::argp_t args = argv + 1;
	
	// Check for sufficient number of args
	if ( *args == NULL )
	{
		static iota::argv_t default_args = { "-", NULL };
		
		args = default_args;
	}
	
	// Print each file in turn.  Return whether any errors occurred.
	int exit_status = EXIT_SUCCESS;
	
	while ( *args != NULL )
	{
		const char* pathname = EvaluateMetaFilename( *args++ );
		
		try
		{
			NN::Owned< p7::fd_t > fd = p7::open( pathname, p7::o_rdonly );
			
			p7::pump( fd, NULL, p7::stdout_fileno, NULL, 0 );
		}
		catch ( const p7::errno_t& error )
		{
			std::fprintf( stderr, "%s: %s: %s\n", argv0, pathname, std::strerror( error ) );
			
			exit_status = EXIT_FAILURE;
			
			continue;
		}
	}
	
	return exit_status;
}

