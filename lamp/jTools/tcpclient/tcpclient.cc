/*	============
 *	tcpclient.cc
 *	============
 */

// Standard C/C++
#include <cerrno>

// POSIX
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>

// Iota
#include "iota/strings.hh"

// gear
#include "gear/inscribe_decimal.hh"
#include "gear/parse_decimal.hh"

// plus
#include "plus/var_string.hh"

// poseven
#include "poseven/bundles/inet.hh"
#include "poseven/functions/dup2.hh"
#include "poseven/functions/execvp.hh"
#include "poseven/functions/socket.hh"
#include "poseven/functions/write.hh"
#include "poseven/types/exit_t.hh"

// Orion
#include "Orion/Main.hh"


namespace tool
{
	
	namespace p7 = poseven;
	
	
	static p7::in_addr_t ResolveHostname( const char* hostname )
	{
		hostent* hosts = gethostbyname( hostname );
		
		if ( !hosts || h_errno )
		{
			plus::var_string message = "Domain name lookup failed: ";
			
			message += gear::inscribe_decimal( h_errno );
			message += "\n";
			
			p7::write( p7::stderr_fileno, message );
			
			throw p7::exit_failure;
		}
		
		in_addr addr = *(in_addr*) hosts->h_addr;
		
		return p7::in_addr_t( addr.s_addr );
	}
	
	static void Connect( const char* hostname, const char* port_str )
	{
		p7::in_port_t port = p7::in_port_t( gear::parse_unsigned_decimal( port_str ) );
		
		p7::in_addr_t addr = ResolveHostname( hostname );
		
		const p7::fd_t tcp_in  = p7::fd_t( 6 );
		const p7::fd_t tcp_out = p7::fd_t( 7 );
		
		p7::dup2( p7::connect( addr, port ), tcp_in );
		
		p7::dup2( tcp_in, tcp_out );
	}
	
	int Main( int argc, char** argv )
	{
		if ( argc < 4 )
		{
			p7::write( p7::stderr_fileno, STR_LEN( "Usage:  tcpclient <host> <port> <program argv>\n" ) );
			
			return 1;
		}
		
		const char* hostname = argv[1];
		const char* port_str = argv[2];
		
		char** program_argv = argv + 3;
		
		Connect( hostname, port_str );
		
		p7::execvp( program_argv );
		
		// Not reached
		return 0;
	}
	
}

