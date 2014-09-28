/*	=============
 *	test-pread.cc
 *	=============
 */

// Standard C
#include <string.h>

// POSIX
#include <unistd.h>

// Iota
#include "iota/strings.hh"

// tap-out
#include "tap/test.hh"

// poseven
#include "poseven/functions/lseek.hh"
#include "poseven/functions/open.hh"
#include "poseven/functions/unlink.hh"
#include "poseven/functions/write.hh"

// Orion
#include "Orion/Main.hh"


namespace tool
{
	
	namespace n = nucleus;
	namespace p7 = poseven;
	
	
	static const unsigned n_tests = 7;
	
	
	static void pread_pipe()
	{
		int fds[2];
		
		p7::throw_posix_result( pipe( fds ) );
		
		char buffer[ 16 ];
		
		int n_read = pread( fds[0], buffer, sizeof buffer, 0 );
		
		EXPECT( n_read == -1  &&  errno == ESPIPE );
		
		close( fds[0] );
		close( fds[1] );
	}
	
	static const char *const tmp_file_path = "/tmp/pread.txt";
	
	static void create_tmp_file()
	{
		p7::write( p7::open( tmp_file_path,
		                     p7::o_rdwr | p7::o_creat | p7::o_trunc ),
		           STR_LEN( "123456789abcdef\n" ) );
	}
	
	static void pread_file()
	{
		n::owned< p7::fd_t > tmp_file = p7::open( tmp_file_path, p7::o_rdonly );
		
		char buffer[ 16 ];
		
		
		int length = STRLEN( "1234" );
		
		int n_read = pread( tmp_file, buffer, length, 0 );
		
		EXPECT( n_read == length  &&  memcmp( buffer, STR_LEN( "1234" ) ) == 0 );
		
		EXPECT( p7::lseek( tmp_file ) == 0 );
		
		
		length = STRLEN( "34567" );
		
		n_read = pread( tmp_file, buffer, length, 2 );
		
		EXPECT( n_read == length  &&  memcmp( buffer, STR_LEN( "34567" ) ) == 0 );
		
		EXPECT( p7::lseek( tmp_file ) == 0 );
		
		
		p7::lseek( tmp_file, 6 );
		
		length = STRLEN( "cdef\n" );
		
		n_read = pread( tmp_file, buffer, length + 3, 11 );
		
		EXPECT( n_read == length  &&  memcmp( buffer, STR_LEN( "cdef\n" ) ) == 0 );
		
		EXPECT( p7::lseek( tmp_file ) == 6 );
	}
	
	static void delete_tmp_file()
	{
		p7::unlink( tmp_file_path );
	}
	
	int Main( int argc, char** argv )
	{
		tap::start( "test-pread", 7 );
		
		pread_pipe();
		
		create_tmp_file();
		
		pread_file();
		
		delete_tmp_file();
		
		return 0;
	}
	
}
