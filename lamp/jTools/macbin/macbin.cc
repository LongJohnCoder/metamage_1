/*	=========
 *	macbin.cc
 *	=========
 */

// Standard C/C++
#include <cstdio>

// Standard C++
#include <functional>
#include <vector>

// POSIX
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

// Iota
#include "iota/strings.hh"

// Nucleus
#include "Nucleus/NAssert.h"

// Nitrogen
#include "Nitrogen/OSStatus.h"

// POSeven
#include "POSeven/Open.hh"
#include "POSeven/Stat.hh"

// Divergence
#include "Divergence/Utilities.hh"

// Misc
#include "MacBinary.hh"

// Orion
#include "Orion/GetOptions.hh"
#include "Orion/Main.hh"


namespace N = Nitrogen;
namespace NN = Nucleus;
namespace p7 = poseven;
namespace Div = Divergence;
namespace O = Orion;


// Operations:
// 
// Foo must be a real (possibly nonexisting) file (not a device stream)
// 
// --encode Foo foo.out   # If foo.out is omitted, a name is chosen.
// --decode foo.in Foo    # If Foo is omitted, the MacBinary-specified name is used.
// --check foo.in         # tests whether or not foo.in is a MacBinary envelope.


static int Usage()
{
	p7::write( p7::stderr_fileno, STR_LEN( "Usage: macbin --encode Source [dest.mbin]\n"
	                                       "       macbin --decode source.bin [dest-dir]\n" ) );
	
	return 1;
}

static void BlockWrite( int fd, const void* data, std::size_t byteCount )
{
	p7::write( p7::fd_t( fd ), (const char*) data, byteCount );
}

static void Decode( p7::fd_t input, const N::FSDirSpec& destDir )
{
	MacBinary::Decoder decoder( destDir );
	
	const std::size_t blockSize = 4096;
	
	char data[ blockSize ];
	
	std::size_t totalBytes = 0;
	
	try
	{
		while ( std::size_t bytes = p7::read( input, data, blockSize ) )
		{
			decoder.Write( data, bytes );
			
			totalBytes += bytes;
		}
	}
	catch ( const MacBinary::InvalidMacBinaryHeader& )
	{
		std::fprintf( stderr, "Invalid MacBinary header somewhere past offset %x\n", totalBytes );
		O::ThrowExitStatus( 1 );
	}
}

static std::string make_archive_name( std::string name )
{
	const char* extension = ".jbin";
	
	if ( *name.rbegin() == '/' )
	{
		name.resize( name.size() - 1 );
	}
	else if ( io::file_exists( name ) )
	{
		extension = ".mbin";
	}
	
	return name + extension;
}

int O::Main( int argc, argv_t argv )
{
	NN::RegisterExceptionConversion< NN::Exception, N::OSStatus >();
	
	const char* encode_target = NULL;
	const char* decode_target = NULL;
	
	O::BindOption( "--encode", encode_target );
	O::BindOption( "--decode", decode_target );
	
	O::GetOptions( argc, argv );
	
	char const *const *freeArgs = O::FreeArguments();
	
	std::size_t argCount = O::FreeArgumentCount();
	
	if ( bool both_or_neither = (encode_target == NULL) == (decode_target == NULL) )
	{
		// There can be only one!
		return Usage();
	}
	
	if ( encode_target )
	{
		// FIXME:  Can't encode to a non-file stream, including stdout
		FSSpec targetFile = Div::ResolvePathToFSSpec( encode_target );
		
		MacBinary::Encode( targetFile,
		                   &BlockWrite,
		                   p7::open( argCount > 0 ? freeArgs[ 0 ] : make_archive_name( encode_target ).c_str(),
		                             p7::o_wronly | p7::o_excl | p7::o_creat,
		                             0644 ) );
	}
	else if ( decode_target )
	{
		const char* destDirPath = argCount > 0 ? freeArgs[ 0 ] : ".";
		
		try
		{
			if ( bool use_stdin = decode_target[0] == '-'  &&  decode_target[1] == '\0' )
			{
				decode_target = "/dev/fd/0";
			}
			
			Decode( io::open_for_reading( decode_target ),
			        NN::Convert< N::FSDirSpec >( Div::ResolvePathToFSSpec( destDirPath ) ) );
		}
		catch ( const MacBinary::InvalidMacBinaryHeader& )
		{
			std::fprintf( stderr, "macbin: %s: invalid MacBinary header\n", decode_target );
			return 1;
		}
		catch ( const MacBinary::IncompatibleMacBinaryHeader& )
		{
			std::fprintf( stderr, "macbin: %s: incompatible (newer) MacBinary header\n", decode_target );
			return 2;
		}
	}
	else
	{
		return Usage();
	}
	
	return 0;
}

