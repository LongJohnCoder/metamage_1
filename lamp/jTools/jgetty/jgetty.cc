/*	=========
 *	jgetty.cc
 *	=========
 */

// Iota
#include "iota/strings.hh"

// POSeven
#include "POSeven/extras/spew.hh"
#include "POSeven/functions/chdir.hh"
#include "POSeven/functions/execv.hh"
#include "POSeven/functions/ioctl.hh"
#include "POSeven/functions/link.hh"
#include "POSeven/functions/open.hh"
#include "POSeven/functions/rename.hh"
#include "POSeven/functions/setsid.hh"
#include "POSeven/functions/symlink.hh"
#include "POSeven/functions/utime.hh"
#include "POSeven/functions/vfork.hh"
#include "POSeven/functions/wait.hh"


namespace NN = Nucleus;
namespace p7 = poseven;


int main( int argc, char const *const argv[] )
{
	p7::chdir( "/new/window" );
	
	const short width  = 2 * 4 +  6 * 80 + 15;
	const short height = 2 * 4 + 11 * 24;
	
	p7::spew( "size", STR_LEN( "503x272" "\n" ) );
	
	p7::utime( "ref" );
	
	p7::spew( "ref/text-font", STR_LEN( "4" "\n" ) );
	p7::spew( "ref/text-size", STR_LEN( "9" "\n" ) );
	
	p7::link( "/new/scrollframe", "view"     );
	p7::link( "/new/frame",       "view/v"   );
	p7::link( "/new/console",     "view/v/v" );
	
	p7::symlink( "v/v", "view/target" );
	
	p7::spew( "view/vertical", STR_LEN( "1" "\n" ) );
	p7::spew( "view/v/margin", STR_LEN( "4" "\n" ) );
	
	p7::rename( "view/v/v/tty", "tty" );
	
	NN::Owned< p7::fd_t > tty = p7::open( "tty", p7::o_rdwr );
	
	dup2( tty, p7::stdin_fileno  );
	dup2( tty, p7::stdout_fileno );
	dup2( tty, p7::stderr_fileno );
	
	if ( tty.get() > p7::stderr_fileno )
	{
		p7::close( tty );
	}
	
	p7::pid_t sid = p7::setsid();  // New session
	
	p7::ioctl( p7::stdin_fileno, TIOCSCTTY, NULL );  // Reattach to terminal
	
	if ( const char* name = ttyname( p7::stdin_fileno ) )
	{
		std::string title = name;
		
		p7::spew( "title", title + "\n" );
	}
	
	p7::pid_t pid = p7::vfork();
	
	if ( pid == 0 )
	{
		p7::chdir( "/" );
		
		char* login_argv[] = { "/bin/login", NULL };
		
		p7::execv( login_argv[0], login_argv );
	}
	
	p7::wait_t wait_status = p7::wait();
	
	// If the child exited unsuccessfully and the terminal is still connected,
	// keep the window around.
	if ( wait_status != 0  &&  ::write( p7::stdout_fileno, "", 0 ) == 0 )
	{
		// Reset ourselves as the foreground process group of the terminal,
		// so we'll get SIGHUP when the window is closed.
		int pgrpset = tcsetpgrp( p7::stdin_fileno, sid );
		
		p7::spew( "title", STR_LEN( "(Done)" "\n" ) );
		
		// Loop so resizing the window doesn't kill us (via SIGWINCH).
		while ( true )
		{
			pause();
		}
	}
	
	return 0;
}

