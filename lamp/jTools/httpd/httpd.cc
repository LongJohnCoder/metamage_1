/*	========
 *	httpd.cc
 *	========
 */

// Standard C++
#include <numeric>
#include <map>
#include <string>
#include <vector>

// Standard C/C++
#include <cctype>
#include <cstdlib>
#include <cstring>

// Standard C
#include <stdlib.h>

// POSIX
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

// Iota
#include "iota/strings.hh"

// Nucleus
#include "Nucleus/NAssert.h"

// POSeven
#include "POSeven/Directory.hh"
#include "POSeven/Open.hh"
#include "POSeven/Pathnames.hh"
#include "POSeven/Stat.hh"
#include "POSeven/extras/pump.hh"

// Nitrogen
#include "Nitrogen/Files.h"

// BitsAndBytes
#include "HexStrings.hh"

// Arcana
#include "HTTP.hh"

// Divergence
#if TARGET_OS_MAC
#include "Divergence/Utilities.hh"
#endif

// Orion
#include "Orion/GetOptions.hh"
#include "Orion/Main.hh"


#define HTTP_VERSION  "HTTP/1.0"


namespace tool
{
	
	namespace N = Nitrogen;
	namespace NN = Nucleus;
	namespace p7 = poseven;
	namespace Div = Divergence;
	namespace O = Orion;
	
	
	using namespace io::path_descent_operators;
	
	using BitsAndBytes::EncodeAsHex;
	
	
	static const char* gDocumentRoot = "/var/www";
	
	
	static char ToCGI( char c )
	{
		return c == '-' ? '_' : std::toupper( c );
	}
	
	static void SetCGIVariables( const HTTP::MessageReceiver& request )
	{
		const HTTP::HeaderIndex& index = request.GetHeaderIndex();
		
		const char* stream = request.GetHeaderStream();
		
		for ( HTTP::HeaderIndex::const_iterator it = index.begin();  it != index.end();  ++it )
		{
			std::string name( stream + it->field_offset,
			                  stream + it->colon_offset );
			
			std::string value( stream + it->value_offset,
			                   stream + it->crlf_offset );
			
			std::transform( name.begin(),
			                name.end(),
			                name.begin(),
			                std::ptr_fun( ToCGI ) );
			
			if ( name == "CONTENT_TYPE"  ||  name == "CONTENT_LENGTH" )
			{
			}
			else
			{
				name = "HTTP_" + name;
			}
			
			setenv( name.c_str(), value.c_str(), 1 );
		}
	}
	
	static int ForkExecWait( const char*                   path,
	                         char const* const             argv[],
	                         const HTTP::MessageReceiver&  request )
	{
		const std::string& partialData = request.GetPartialContent();
		
		bool partial_data_exist = !partialData.empty();
		
		int pipe_ends[2];
		
		if ( partial_data_exist )
		{
			pipe( pipe_ends );
		}
		
		p7::fd_t reader = p7::fd_t( pipe_ends[0] );
		p7::fd_t writer = p7::fd_t( pipe_ends[1] );
		
		int pid = vfork();
		
		if ( pid == -1 )
		{
			std::perror( "httpd: vfork()" );
			
			return -1;
		}
		
		if ( pid == 0 )
		{
			if ( partial_data_exist )
			{
				close( writer );
				
				dup2( reader, p7::stdin_fileno );  // read from pipe instead of socket
				
				close( reader );
			}
			
			// This eliminates LOCAL_EDITOR, PATH, and SECURITYSESSIONID.
			// We'll have to consider other approaches.
			//clearenv();
			
			SetCGIVariables( request );
			
			execv( path, const_cast< char* const* >( argv ) );
			
			std::string message = "httpd: ";
			
			message += path;
			
			std::perror( message.c_str() );
			
			_exit( 1 );  // Use _exit() to exit a forked but not exec'ed process.
		}
		
		if ( partial_data_exist )
		{
			close( reader );
			
			p7::write( writer, partialData.data(), partialData.size() );
			
			p7::pump( p7::stdin_fileno, writer );
			
			close( writer );
		}
		
		int stat = -1;
		pid_t resultpid = waitpid( pid, &stat, 0 );
		
		if ( resultpid == -1 )
		{
			std::perror( "httpd: waitpid()" );
		}
		
		return stat;
	}
	
	
	struct ParsedRequest
	{
		std::string method;
		std::string resource;
		std::string version;
	};
	
	static ParsedRequest ParseRequest( const std::string& request )
	{
		// E.g.  "GET / HTTP/1.0"
		
		ParsedRequest parsed;
		
		// Find the first space (which ends the request method)
		std::string::size_type end = request.find( ' ' );
		
		parsed.method = request.substr( 0, end - 0 );  // e.g. "GET"
		
		// The resource starts after the space
		std::string::size_type resource = end + 1;
		
		// and ends with the next space
		end = request.find( ' ', resource );
		
		parsed.resource = request.substr( resource, end - resource );  // e.g. "/logo.png"
		
		// HTTP version string starts after the second space
		std::string::size_type version = end + 1;
		// and runs to the end
		end = std::string::npos;
		
		parsed.version = request.substr( version, end - version );  // e.g. "HTTP/1.1"
		
		return parsed;
	}
	
	class ResourceParser
	{
		private:
			std::string resource;
		
		public:
			class const_iterator
			{
				private:
					const std::string& s;
					std::string::size_type pos, end;
				
				public:
					enum { npos = std::string::npos };
					struct End  {};
					static End kEnd;
					
					const_iterator( const std::string& s )
					:
						s  ( s                  ),
						pos( 1                  ),
						end( s.find( '/', pos ) )
					{}
					
					const_iterator( const std::string& s, End )
					:
						s  ( s    ),
						pos( npos ),
						end( npos )
					{}
					
					std::string operator*() const  { return s.substr( pos, end - pos ); }
					
					const_iterator& operator++()
					{
						if ( end + 1 < s.size() )
						{
							pos = end + 1;
							end = s.find( '/' , pos );
						}
						else
						{
							pos = end = std::string::npos;
						}
						
						return *this;
					}
					
					const_iterator operator++( int )
					{
						const_iterator old = *this;
						++*this;
						return old;
					}
					
					friend bool operator==( const const_iterator& a, const const_iterator& b )
					{
						return a.pos == b.pos
							&& a.end == b.end;
					}
					
					friend bool operator!=( const const_iterator& a, const const_iterator& b )
					{
						return !( a == b );
					}
			};
			
			ResourceParser( const std::string& resource ) : resource( resource + "/" )  {}
			
			const_iterator begin() const  { return const_iterator( resource                       ); }
			const_iterator end()   const  { return const_iterator( resource, const_iterator::kEnd ); }
	};
	
	inline char check_xdigit( char c )
	{
		if ( !std::isxdigit( c ) )
		{
			p7::throw_errno( ENOENT );  // FIXME
		}
		
		return c;
	}
	
	static std::string expand_percent_escapes( const std::string& escaped )
	{
		using BitsAndBytes::DecodeHexNibbles;
		
		std::string result;
		
		//const char* p = &*escaped.begin();
		const char* end = &*escaped.end();
		
		for ( const char* p = &*escaped.begin();  p < end;  ++p )
		{
			char c = *p;
			
			if ( c == '%' )
			{
				char high = check_xdigit( *++p );
				char low  = check_xdigit( *++p );
				
				c = DecodeHexNibbles( high, low );
			}
			
			result += c;
		}
		
		return result;
	}
	
	static std::string LocateResource( const std::string& resource )
	{
		ResourceParser parser( resource );
		
		std::string pathname = gDocumentRoot;
		
		// FIXME:  This can be an algorithm
		typedef ResourceParser::const_iterator const_iterator;
		for ( const_iterator it = parser.begin();  it != parser.end();  ++it )
		{
			std::string component = expand_percent_escapes( *it );
			
			if ( !component.empty() && component[0] == '.' )
			{
				// No screwing around with the pathname, please.
				p7::throw_errno( ENOENT );
			}
			
			pathname = pathname / component;
		}
		
		p7::stat( pathname );  // Throw if nonexistent
		
		return pathname;
	}
	
	static std::string FilenameExtension(const std::string& filename)
	{
		// This will break for index.html.en and similar
		std::string::size_type lastDot = filename.find_last_of( "." );
		
		if ( lastDot != std::string::npos )
		{
			return filename.substr( lastDot, std::string::npos );
		}
		else
		{
			return "";
		}
	}
	
	static std::string FilenameExtension(const unsigned char* filename)
	{
		return FilenameExtension( NN::Convert< std::string >( filename ) );
	}
	
	static std::string GuessContentType( const std::string& filename, ::OSType type )
	{
		std::string ext = FilenameExtension( filename );
		
		if ( ext == ".html" )
		{
			return "text/html";
		}
		else if ( ext == ".css" )
		{
			return "text/css";
		}
		else if ( type == 'TEXT' )
		{
			return "text/plain";
		}
		else if ( type == 'BINA' )
		{
			return "application/x-macbinary";
		}
		
		return "application/octet-stream";
	}
	
	static void DumpFile( const std::string& pathname )
	{
		HTTP::SendMessageBody( p7::stdout_fileno, io::open_for_reading( pathname ) );
	}
	
	static void ListDir( const std::string& pathname )
	{
		typedef io::directory_contents_traits< std::string >::container_type directory_container;
		
		directory_container contents = io::directory_contents( pathname );
		
		typedef directory_container::const_iterator Iter;
		
		for ( Iter it = contents.begin();  it != contents.end();  ++it )
		{
			std::string listing = io::get_filename_string( *it );
			
			listing += "\n";
			
			io::write( p7::stdout_fileno, listing.data(), listing.size() );
		}
		
	}
	
	static void SendError( const std::string& error )
	{
		std::string message = HTTP_VERSION " " + error +      "\r\n"
		                      "Content-Type: text/html"       "\r\n"
		                                                      "\r\n"
		                      "<title>" + error + "</title>"  "\r\n"
		                      "<p>"     + error + "</p>"      "\r\n";
		
			io::write( p7::stdout_fileno, message.data(), message.size() );
	}
	
	static void SendResponse( const HTTP::MessageReceiver& request )
	{
		ParsedRequest parsed = ParseRequest( request.GetStatusLine() );
		
		std::string pathname;
		
		try
		{
			pathname = LocateResource( parsed.resource );
		}
		catch ( ... )
		{
			p7::write( p7::stdout_fileno,
			           STR_LEN( HTTP_VERSION " 404 Not Found"                "\r\n"
			                                 "Content-Type: text/html"       "\r\n"
			                                                                 "\r\n"
			                                 "<title>404 Not Found</title>"  "\r\n"
			                                 "<p>404 Not Found</p>"          "\r\n" ) );
			
			return;
		}
		
		std::string cgiBin = "/cgi-bin/";
		
		if ( parsed.resource.substr( 0, cgiBin.size() ) == cgiBin )
		{
			const char* path = pathname.c_str();
			
			char const* const argv[] = { path, NULL };
			
			int stat = ForkExecWait( path, argv, request );
		}
		else
		{
			bool is_dir = false;
			
			OSType type = kUnknownType;
			
			if ( io::directory_exists( pathname ) )
			{
				if ( *pathname.rbegin() != '/' )
				{
					SendError( "404 Not Found" );
					
					return;
				}
				
				std::string index_html = pathname / "index.html";
				
				if ( io::file_exists( index_html ) )
				{
					pathname = index_html;
				}
				else
				{
					is_dir = true;
				}
			}
			
			FInfo info = { 0 };
			
		#if TARGET_OS_MAC
			
			if ( !is_dir )
			{
				FSSpec file = Div::ResolvePathToFSSpec( pathname );
				
				info = N::FSpGetFInfo( file );
				
				type = info.fdType;
			}
			
		#endif
			
			std::string contentType = is_dir ? "text/plain" : GuessContentType( pathname, type );
			
			std::string responseHeader = HTTP_VERSION " 200 OK\r\n";
			
			responseHeader += HTTP::HeaderFieldLine( "Content-Type",  contentType                   );
			
		#if TARGET_OS_MAC
			
			responseHeader += HTTP::HeaderFieldLine( "X-Mac-Type",    EncodeAsHex( info.fdType    ) );
			responseHeader += HTTP::HeaderFieldLine( "X-Mac-Creator", EncodeAsHex( info.fdCreator ) );
			
		#endif
			
			responseHeader += "\r\n";
			
			p7::write( p7::stdout_fileno, responseHeader.data(), responseHeader.size() );
			
			if ( parsed.method != "HEAD" )
			{
				is_dir ? ListDir( pathname ) : DumpFile( pathname );
			}
		}
	}
	
	int Main( int argc, iota::argv_t argv )
	{
		O::BindOption( "--doc-root", gDocumentRoot );
		
		O::GetOptions( argc, argv );
		
		sockaddr_in peer;
		socklen_t peerlen = sizeof peer;
		
		if ( getpeername( 0, (sockaddr*)&peer, &peerlen ) == 0 )
		{
			std::fprintf( stderr, "Connection from %s, port %d\n",
			                                       inet_ntoa( peer.sin_addr ),
			                                                peer.sin_port );
		}
		
		HTTP::MessageReceiver request;
		
		request.ReceiveHeader( p7::stdin_fileno );
		
		SendResponse( request );
		
		return 0;
	}

}

