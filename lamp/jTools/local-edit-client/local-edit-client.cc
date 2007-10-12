/*	====================
 *	local-edit-client.cc
 *	====================
 */

// Standard C/C++
#include <cerrno>
#include <cstdlib>

// Standard C++
#include <string>

// POSIX
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

// Iota
#include "iota/strings.hh"

// POSeven
#include "POSeven/Errno.hh"
#include "POSeven/FileDescriptor.hh"
#include "POSeven/Open.hh"
#include "POSeven/Pathnames.hh"

// Arcana
#include "HTTP.hh"
#include "MD5.hh"

// Orion
#include "Orion/GetOptions.hh"
#include "Orion/Main.hh"


namespace NN = Nucleus;
namespace p7 = poseven;
namespace O = Orion;


static MD5::Result MD5DigestFile( p7::fd_t input )
{
	MD5::Engine md5;
	
	const std::size_t blockSize = 4096;
	
	char buffer[ blockSize ];
	
	while ( std::size_t bytes_read = p7::read( input, buffer, blockSize ) )
	{
		if ( bytes_read == blockSize )
		{
			for ( const char* p = buffer;  p < buffer + blockSize;  p += 64 )
			{
				md5.DoBlock( p );
			}
		}
		else
		{
			const char* end_of_blocks = buffer + bytes_read - bytes_read % 64;
			
			for ( const char* p = buffer;  p < end_of_blocks;  p += 64 )
			{
				md5.DoBlock( p );
			}
			
			md5.Finish( end_of_blocks, bytes_read % 64 * 8 );
			
			return md5.GetResult();
		}
	}
	
	md5.Finish( buffer, 0 );
	
	return md5.GetResult();
}

static void EncodeBase64Triplet( const unsigned char* triplet, unsigned char* buffer )
{
	const char* code = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	
	const unsigned char mask = 0x3f;
	
	buffer[0] = code[                            (triplet[0] >> 2) ];
	buffer[1] = code[ mask & (triplet[0] << 4) | (triplet[1] >> 4) ];
	buffer[2] = code[ mask & (triplet[1] << 2) | (triplet[2] >> 6) ];
	buffer[3] = code[ mask & (triplet[2] << 0)                     ];
}

static std::string EncodeBase64( const unsigned char* begin, const unsigned char* end )
{
	std::string result;
	
	result.reserve( (end - begin + 2) * 4 / 3 );
	
	unsigned char buffer[4];
	
	while ( begin < end - 2 )
	{
		EncodeBase64Triplet( begin, buffer );
		
		result.append( (char*) buffer, 4 );
		
		begin += 3;
	}
	
	if ( begin < end )
	{
		unsigned char final[3] = { 0, 0, 0 };
		
		std::copy( begin, end, final );
		
		EncodeBase64Triplet( final, buffer );
		
		if ( end - begin == 1 )
		{
			buffer[2] = '=';
		}
		
		buffer[3] = '=';
		
		result.append( (char*) buffer, 4 );
	}
	
	return result;
}

static void Splice( p7::fd_t in, p7::fd_t out )
{
	const std::size_t block_size = 4096;
	
	char buffer[ block_size ];
	
	while ( int bytes_read = read( in, buffer, block_size ) )
	{
		if ( bytes_read == -1 )
		{
			std::perror( "local-edit-client: read" );
			
			p7::throw_errno( errno );
		}
		
		p7::write( out, buffer, bytes_read );
	}
}

static void CopyFileContents( p7::fd_t in, p7::fd_t out )
{
	p7::throw_posix_result( lseek( in,  0, 0 ) );
	p7::throw_posix_result( lseek( out, 0, 0 ) );
	
	p7::throw_posix_result( ftruncate( out, 0 ) );
	
	Splice( in, out );
}

static void CommitFileEdits( p7::fd_t  edited_file_stream,
                             p7::fd_t  target_file_stream )
{
	CopyFileContents( edited_file_stream, target_file_stream );
}

static void CommitFileEditsWithBackup( p7::fd_t  edited_file_stream,
                                       p7::fd_t  target_file_stream,
                                       p7::fd_t  backup_file_stream )
{
	if ( backup_file_stream != -1 )
	{
		CopyFileContents( target_file_stream, backup_file_stream );
	}
	
	CommitFileEdits( edited_file_stream, target_file_stream );
}


int O::Main( int argc, argv_t argv )
{
	bool dumpHeaders = false;
	
	char const *const defaultOutput = "/dev/fd/1";
	
	const char* outputFile = defaultOutput;
	
	O::BindOption( "-i", dumpHeaders );
	O::BindOption( "-o", outputFile  );
	
	O::GetOptions( argc, argv );
	
	char const *const *freeArgs = O::FreeArguments();
	
	std::size_t argCount = O::FreeArgumentCount();
	
	const char* target_pathname = argCount > 0 ? freeArgs[0] : "/dev/null";
	
	NN::Owned< p7::fd_t > target_file_stream = p7::open( target_pathname, p7::o_rdwr | p7::o_creat );
	
	MD5::Result digest = MD5DigestFile( target_file_stream );
	
	std::string old_digest_b64 = EncodeBase64( digest.data, digest.data + 16 );
	
	//p7::lseek( target_file_stream, 0, 0 );
	lseek( target_file_stream, 0, 0 );
	
	const p7::fd_t socket_in  = p7::fd_t( 6 );
	const p7::fd_t socket_out = p7::fd_t( 7 );
	
	const char* method = "APPLY";
	
	const char* urlPath = "/cgi-bin/local-edit-server";
	
	std::string contentLengthHeader;
	
	try
	{
		contentLengthHeader = HTTP::GetContentLengthHeaderLine( target_file_stream );
	}
	catch ( ... )
	{
	}
	
	std::string message_header =   HTTP::RequestLine( method, urlPath )
	                             //+ HTTP::HeaderLine( "Host", hostname )
	                             + HTTP::HeaderLine( "X-Edit-Title", io::get_filename( target_pathname ) )
	                             + HTTP::HeaderLine( "Content-MD5", old_digest_b64 )
	                             + contentLengthHeader
	                             + "\r\n";
	
	HTTP::SendMessage( socket_out, message_header, target_file_stream );
	
	shutdown( socket_out, SHUT_WR );
	
	HTTP::ResponseReceiver response;
	
	response.ReceiveHeaders( socket_in );
	
	if ( dumpHeaders )
	{
		const std::string& message = response.GetMessageStream();
		
		p7::write( p7::stdout_fileno, message.data(), message.size() );
	}
	
	unsigned result_code = response.GetResultCode();
	
	if ( result_code == 200 )
	{
		NN::Owned< p7::fd_t > edited_file_stream = p7::open( outputFile, p7::o_rdwr | p7::o_trunc | p7::o_creat, 0400 );
		
		const std::string& partial_content = response.GetPartialContent();
		
		if ( !partial_content.empty() )
		{
			p7::write( edited_file_stream, partial_content.data(), partial_content.size() );
		}
		
		HTTP::SendMessageBody( edited_file_stream, socket_in );
		
		lseek( edited_file_stream, 0, 0 );
		
		digest = MD5DigestFile( edited_file_stream );
		
		std::string new_digest_b64 = EncodeBase64( digest.data, digest.data + 16 );
		
		std::string received_digest_b64 = response.GetHeader( "Content-MD5", "" );
		
		if ( new_digest_b64 != received_digest_b64 )
		{
			std::fprintf( stderr, "MD5 digest mismatch\n" );
			
			unlink( outputFile );
			
			return EXIT_FAILURE;
		}
		
		// FIXME:  test for a regular file
		if ( outputFile != defaultOutput )
		{
			p7::write( p7::stdout_fileno, STR_LEN( "Hit return to confirm or Control-D to cancel: " ) );
			
			while ( true )
			{
				char c;
				
				int bytes_read = read( p7::stdin_fileno, &c, sizeof c );
				
				if ( bytes_read == -1 )
				{
					std::perror( "local-edit-client: read" );
					
					// I'm not sure what the scenario is here.
					// (EINTR on handled signal?  EIO on disconnected terminal?)
					// Leave tmp file for recovery.
					return EXIT_FAILURE;
				}
				
				if ( bytes_read == 0 )
				{
					p7::write( p7::stdout_fileno, STR_LEN( "\n" "canceled\n" ) );
					
					break;
				}
				
				if ( c == '\n' )
				{
					CommitFileEdits( edited_file_stream, target_file_stream );
					
					break;
				}
			}
			
			p7::close( edited_file_stream );
			
			unlink( outputFile );
		}
		
	}
	else if ( result_code == 304 )
	{
		// Not modified
	}
	else
	{
		std::fprintf( stderr, "%s\n", response.GetResult().c_str() );
		
		return EXIT_FAILURE;
	}
	
	shutdown( socket_in, SHUT_RD );
	
	return 0;
}

