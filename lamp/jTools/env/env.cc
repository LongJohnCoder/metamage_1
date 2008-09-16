/*	======
 *	env.cc
 *	======
 */

// Standard C++
#include <string>

// Standard C/C++
#include <cstring>

// Standard C
#include <stdlib.h>

// POSIX
#include <unistd.h>

// Iota
#include "iota/argv.hh"
#include "iota/environ.hh"


static void DumpEnvironment()
{
	std::string output;
	
	for ( iota::envp_t envp = environ;  *envp != NULL;  ++envp )
	{
		output += *envp;
		output += "\n";
	}
	
	(void) write( STDOUT_FILENO, output.data(), output.size() );
}

int main( int argc, char** argv )
{
	while ( *++argv != NULL  &&  std::strchr( *argv, '=' ) )
	{
		putenv( *argv );
	}
	
	if ( *argv == NULL )
	{
		DumpEnvironment();
	}
	else
	{
		execvp( argv[0], argv );
		
		_exit( 127 );
	}
	
	return 0;
}

