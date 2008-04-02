/*	========
 *	which.cc
 *	========
 */

// Standard C/C++
#include <cstdio>
#include <cstdlib>
#include <cstring>

// Standard C++
#include <string>
#include <vector>

// POSIX
#include <sys/stat.h>
#include <unistd.h>


int main( int argc, char const *const argv[] )
{
	// Check for sufficient number of args
	if ( argc <= 1 )
	{
		return 1;
	}
	
	const char* path = std::getenv( "PATH" );
	
	if ( path == NULL )
	{
		path = "/usr/bin:/bin";
	}
	
	std::vector< std::string > dirs;
	
	for ( const char* p = path;  ;  )
	{
		const char* q = std::strchr( p, ':' );
		
		if ( q == NULL )
		{
			q = std::strchr( p, '\0' );
		}
		
		if ( q > p )
		{
			dirs.push_back( std::string( p, q ) );
		}
		
		if ( *q != ':' )
		{
			break;
		}
		
		p = q + 1;
	}
	
	
	bool failed = false;
	
	for ( const char *const *program = argv + 1;  *program != NULL;  ++program )
	{
		typedef std::vector< std::string >::const_iterator Iter;
		
		bool found = false;
		
		for ( Iter it = dirs.begin();  it != dirs.end();  ++it )
		{
			std::string pathname = *it + '/' + *program;
			
			struct ::stat sb;
			
			int status = ::stat( pathname.c_str(), &sb );
			
			int mask = S_IFMT | S_IXUSR;
			
			if ( status == 0  &&  (sb.st_mode & mask) == (S_IFREG | S_IXUSR) )
			{
				found = true;
				
				std::printf( "%s\n", pathname.c_str() );
				
				break;
			}
		}
		
		if ( !found )
		{
			failed = true;
		}
	}
	
	return failed ? 1 : 0;
}

