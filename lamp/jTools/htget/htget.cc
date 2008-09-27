/*	========
 *	htget.cc
 *	========
 */

// Standard C/C++
#include <cerrno>
#include <cstdlib>

// POSIX
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>

// Iota
#include "iota/strings.hh"

// Nucleus
#include "Nucleus/Convert.h"
#include "Nucleus/NAssert.h"

// POSeven
#include "POSeven/Open.hh"
#include "POSeven/extras/pump.hh"
#include "POSeven/functions/socket.hh"
#include "POSeven/bundles/inet.hh"

// Arcana
#include "HTTP.hh"

// Orion
#include "Orion/GetOptions.hh"
#include "Orion/Main.hh"


namespace tool
{
	
	namespace NN = Nucleus;
	namespace p7 = poseven;
	namespace O = Orion;
	
	
	static bool ParseURL( const std::string& url,
	                      std::string& outURLScheme, 
	                      std::string& outHostname,
	                      std::string& outPort,
	                      std::string& outURLpath )
	{
		std::size_t colonSlashSlash = url.find( "://" );
		
		if ( colonSlashSlash == url.npos )
		{
			return false;
		}
		
		outURLScheme = url.substr( 0, colonSlashSlash );
		
		std::size_t hostnameStart = colonSlashSlash + 3;
		std::size_t slash = url.find( "/", hostnameStart );
		std::size_t colon = url.find( ":", hostnameStart );
		std::size_t hostnameEnd = std::min( slash, colon );
		
		outHostname = url.substr( hostnameStart, hostnameEnd - hostnameStart );
		
		if ( colon < slash )
		{
			outPort = url.substr( colon + 1, slash - (colon + 1) );
		}
		
		outURLpath = (slash == url.npos) ? std::string( "/" ) : url.substr( slash, url.npos );
		
		return true;
	}
	
	
	static const char* DocName( const std::string& urlPath )
	{
		std::size_t lastSlash = urlPath.find_last_of( "/" );
		
		// Skip the slash.
		// If there wasn't one, then lastSlash == string::npos == 0xFFFFFFFF == -1.
		// Adding one then yields zero, which is exactly what we want.
		
		return urlPath.c_str() + lastSlash + 1;
	}
	
	
	static p7::in_addr_t ResolveHostname( const char* hostname )
	{
		hostent* hosts = gethostbyname( hostname );
		
		if ( !hosts || h_errno )
		{
			std::string message = "Domain name lookup failed: ";
			
			message += NN::Convert< std::string >( h_errno );
			message += "\n";
			
			p7::write( p7::stderr_fileno, message );
			
			throw p7::exit_failure;
		}
		
		in_addr addr = *(in_addr*) hosts->h_addr;
		
		return p7::in_addr_t( addr.s_addr );
	}
	
	
	static void receive_document( const std::string&  partial_content,
	                              p7::fd_t            http_server,
	                              p7::fd_t            document_destination )
	{
		p7::write( document_destination, partial_content );
		
		p7::pump( http_server, document_destination );
	}
	
	int Main( int argc, iota::argv_t argv )
	{
		bool sendHEADRequest = false;
		bool dumpHeader      = false;
		bool saveToFile      = false;
		
		const char* defaultOutput = "/dev/fd/1";
		
		const char* outputFile = defaultOutput;
		
		O::BindOption( "-i", dumpHeader      );
		O::BindOption( "-I", sendHEADRequest );
		O::BindOption( "-o", outputFile      );
		O::BindOption( "-O", saveToFile      );
		
		O::AliasOption( "-i", "--headers" );
		O::AliasOption( "-i", "--include" );
		
		O::AliasOption( "-I", "--head" );
		
		O::AliasOption( "-O", "--remote-name" );
		O::AliasOption( "-O", "--save"        );
		
		O::GetOptions( argc, argv );
		
		char const *const *freeArgs = O::FreeArguments();
		
		std::size_t argCount = O::FreeArgumentCount();
		
		if ( argCount == 0 )
		{
			p7::write( p7::stderr_fileno, STR_LEN( "htget: Usage:  htget <url>\n" ) );
			
			return EXIT_FAILURE;
		}
		
		std::string method = "GET";
		
		const bool expecting_content = !sendHEADRequest;
		
		if ( sendHEADRequest )
		{
			dumpHeader = true;
			method = "HEAD";
		}
		
		if ( !expecting_content  &&  (saveToFile  ||  outputFile != defaultOutput) )
		{
			p7::write( p7::stderr_fileno, STR_LEN( "htget: Can't save null document to file\n" ) );
			
			return EXIT_FAILURE;
		}
		
		std::string scheme;
		std::string hostname;
		std::string urlPath;
		std::string portStr;
		
		p7::in_port_t default_port = p7::in_port_t( 0 );
		
		bool parsed = ParseURL( freeArgs[ 0 ], scheme, hostname, portStr, urlPath );
		
		// FIXME:  Eliminate . and .. from urlPath
		
		if ( saveToFile )
		{
			outputFile = DocName( urlPath );
		}
		
		bool outputIsToFile = outputFile != defaultOutput;
		
		if ( scheme == "http" )
		{
			default_port = p7::in_port_t( 80 );
		}
		else
		{
			std::string message = "Unsupported scheme '" + scheme + "'.\n";
			
			p7::write( p7::stderr_fileno, message );
			
			return 2;
		}
		
		p7::in_port_t port = default_port;
		
		if ( !portStr.empty() )
		{
			port = p7::in_port_t( std::atoi( portStr.c_str() ) );
		}
		
		p7::in_addr_t ip = ResolveHostname( hostname.c_str() );
		
		std::string message_header =   HTTP::RequestLine( method, urlPath )
		                             + HTTP::HeaderFieldLine( "Host", hostname )
		                             + "\r\n";
		
		NN::Owned< p7::fd_t > http_server = p7::connect( ip, port );
		
		p7::write( http_server, message_header );
		
		p7::oflag_t create_flags = outputIsToFile ? p7::o_creat | p7::o_excl : p7::oflag_t();
		
		HTTP::ResponseReceiver response;
		
		response.ReceiveHeader( http_server );
		
		if ( dumpHeader )
		{
			p7::write( p7::stdout_fileno, response.GetMessageStream() );
		}
		
		if ( expecting_content )
		{
			receive_document( response.GetPartialContent(),
			                  http_server,
			                  p7::open( outputFile, p7::o_wronly | create_flags, 0644 ) );
		}
		
		return 0;
	}
	
}

