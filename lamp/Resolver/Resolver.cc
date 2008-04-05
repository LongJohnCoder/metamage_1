/*	===========
 *	Resolver.cc
 *	===========
 */

// Standard C/C++
#include <cstdio>

// POSIX
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>


	#pragma mark -
	#pragma mark � arpa/inet �
	
	int inet_aton( const char* name, struct in_addr* out_addr )
	{
		return -1;
	}
	
	char* inet_ntoa( struct in_addr addr )
	{
		static char result[ 16 ] = { 0 };
		
		std::sprintf( result, "%u.%u.%u.%u", (addr.s_addr >> 24) & 0xFF,
		                                     (addr.s_addr >> 16) & 0xFF,
		                                     (addr.s_addr >>  8) & 0xFF,
		                                     (addr.s_addr      ) & 0xFF );
		
		//::OTInetHostToString( addr.s_addr, result );
		
		return result;
	}
	
	#pragma mark -
	#pragma mark � netdb �
	
	extern "C" { int h_errno; }
	
