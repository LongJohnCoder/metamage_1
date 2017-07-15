/*	========
 *	htget.cc
 *	========
 */

// Standard C/C++
#include <cerrno>

// POSIX
#include <fcntl.h>
#include <netdb.h>

// Iota
#include "iota/strings.hh"

// gear
#include "gear/find.hh"
#include "gear/inscribe_decimal.hh"

// poseven
#include "poseven/bundles/inet.hh"
#include "poseven/functions/open.hh"
#include "poseven/functions/read.hh"
#include "poseven/functions/write.hh"

// Arcana
#include "HTTP.hh"

// Orion
#include "Orion/get_options.hh"
#include "Orion/Main.hh"


#ifdef __RELIX__
#define DEFAULT_USER_AGENT  "htget (a MacRelix program)"
#else
#define DEFAULT_USER_AGENT  "htget"
#endif


namespace tool
{
	
	namespace n = nucleus;
	namespace p7 = poseven;
	namespace o = orion;
	
	
	static bool dumping_progress = false;
	
	static size_t content_length = 0;
	
	static plus::string content_length_string;
	
	static void dump_progress( size_t content_bytes_received )
	{
		plus::var_string dump = gear::inscribe_unsigned_decimal( content_bytes_received );
		
		dump += '/';
		
		dump += content_length_string;
		
		dump += '\n';
		
		p7::write( p7::stdout_fileno, dump );
	}
	
	static bool ParseURL( const plus::string& url,
	                      plus::string& outURLScheme, 
	                      plus::string& outHostname,
	                      plus::string& outPort,
	                      plus::string& outURLpath )
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
		
		outURLpath = (slash == url.npos) ? plus::string( "/" ) : url.substr( slash, url.npos );
		
		return true;
	}
	
	
	static inline const char* DocName( const char* url_path, std::size_t length )
	{
		return gear::find_last_match( url_path, length, '/', url_path - 1 ) + 1;
	}
	
	
	static void receive_document( const plus::string&  partial_content,
	                              p7::fd_t             http_server,
	                              p7::fd_t             document_destination )
	{
		size_t n_written = p7::write( document_destination, partial_content );
		
		const size_t buffer_size = 4096;
		
		char buffer[ buffer_size ];
		
		while ( const ssize_t n_read = p7::read( http_server, buffer, buffer_size ) )
		{
			n_written += p7::write( document_destination, buffer, n_read );
			
			if ( dumping_progress )
			{
				dump_progress( n_written );
			}
		}
	}
	
	int Main( int argc, char** argv )
	{
		bool sendHEADRequest = false;
		bool dumpHeader      = false;
		bool saveToFile      = false;
		
		const char* defaultOutput = "/dev/fd/1";
		
		const char* outputFile = defaultOutput;
		
		o::bind_option_to_variable( "-i", dumpHeader      );
		o::bind_option_to_variable( "-I", sendHEADRequest );
		o::bind_option_to_variable( "-o", outputFile      );
		o::bind_option_to_variable( "-O", saveToFile      );
		
		o::bind_option_to_variable( "--dump-progress", dumping_progress );
		
		o::alias_option( "-i", "--headers" );
		o::alias_option( "-i", "--include" );
		
		o::alias_option( "-I", "--head" );
		
		o::alias_option( "-O", "--remote-name" );
		o::alias_option( "-O", "--save"        );
		
		o::get_options( argc, argv );
		
		char const *const *freeArgs = o::free_arguments();
		
		const size_t n_args = o::free_argument_count();
		
		if ( n_args == 0 )
		{
			p7::write( p7::stderr_fileno, STR_LEN( "htget: Usage:  htget <url>\n" ) );
			
			return 1;
		}
		
		const char* method = "GET";
		
		const bool expecting_content = !sendHEADRequest;
		
		if ( sendHEADRequest )
		{
			dumpHeader = true;
			method = "HEAD";
		}
		
		if ( !expecting_content  &&  (saveToFile  ||  outputFile != defaultOutput) )
		{
			p7::write( p7::stderr_fileno, STR_LEN( "htget: Can't save null document to file\n" ) );
			
			return 1;
		}
		
		plus::string scheme;
		plus::string hostname;
		plus::string urlPath;
		plus::string portStr;
		
		const char* default_port = "";
		
		bool parsed = ParseURL( freeArgs[ 0 ], scheme, hostname, portStr, urlPath );
		
		// FIXME:  Eliminate . and .. from urlPath
		
		if ( saveToFile )
		{
			outputFile = DocName( urlPath.c_str(), urlPath.size() );
		}
		
		bool outputIsToFile = outputFile != defaultOutput;
		
		if ( scheme == "http" )
		{
			default_port = "80";
		}
		else
		{
			plus::string message = "Unsupported scheme '" + scheme + "'.\n";
			
			p7::write( p7::stderr_fileno, message );
			
			return 2;
		}
		
		const char* host = hostname.c_str();
		const char* port = default_port;
		
		if ( !portStr.empty() )
		{
			port = portStr.c_str();
		}
		
		plus::string message_header =   HTTP::RequestLine( method, urlPath.c_str(), urlPath.size() )
		                              + HTTP::HeaderFieldLine( "Host", hostname )
		                              + "User-Agent: " DEFAULT_USER_AGENT "\r\n"
		                              + "\r\n";
		
		n::owned< p7::fd_t > http_server = p7::connect( host, port );
		
		p7::write( http_server, message_header );
		
		p7::open_flags_t create_flags = outputIsToFile ? p7::o_creat | p7::o_excl : p7::open_flags_t();
		
		HTTP::ResponseReceiver response;
		
		response.ReceiveHeader( http_server );
		
		if ( dumpHeader )
		{
			p7::write( p7::stdout_fileno, response.GetMessageStream() );
		}
		
		if ( expecting_content )
		{
			const plus::string& partial_content = response.GetPartialContent();
			
			if ( dumping_progress )
			{
				content_length = response.ContentLengthOrZero();
				
				if ( content_length )
				{
					content_length_string = gear::inscribe_unsigned_decimal( content_length );
					
					dump_progress( partial_content.size() );
				}
			}
			
			receive_document( partial_content,
			                  http_server,
			                  p7::open( outputFile, p7::o_wronly | create_flags ) );
		}
		
		return 0;
	}
	
}
