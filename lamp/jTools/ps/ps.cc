/*	=====
 *	ps.cc
 *	=====
 */

// POSIX
#include <dirent.h>

// iota
#include "iota/decimal.hh"
#include "iota/strings.hh"

// gear
#include "gear/parse_float.hh"

// Debug
#include "debug/assert.hh"

// plus
#include "plus/var_string.hh"

// poseven
#include "poseven/functions/ftruncate.hh"
#include "poseven/functions/open.hh"
#include "poseven/functions/openat.hh"
#include "poseven/functions/pwrite.hh"
#include "poseven/functions/read.hh"
#include "poseven/functions/write.hh"

// Orion
#include "Orion/get_options.hh"
#include "Orion/Main.hh"


static struct timespec timespec_from_seconds( float time )
{
	const unsigned long seconds     = time;
	const unsigned long nanoseconds = (time - seconds) * 1000 * 1000 * 1000;
	
	const struct timespec result = { seconds, nanoseconds };
	
	return result;
}

namespace tool
{
	
	namespace n = nucleus;
	namespace p7 = poseven;
	namespace o = orion;
	
	
	static bool globally_wide = false;
	
	static p7::fd_t g_proc = p7::open( "/proc", p7::o_rdonly | p7::o_directory ).release();
	
	
	static plus::string left_padded( const char* begin, const char* end, unsigned length )
	{
		length = std::max< unsigned >( length, end - begin );
		
		const size_t pad_length = length - (end - begin);
		
		plus::var_string result;
		
		result.reserve( length );
		
		result.resize( pad_length, ' ' );
		
		result.append( begin, end );
		
		return result;
	}
	
	static plus::string right_padded( const char* begin, const char* end, unsigned length )
	{
		length = std::max< unsigned >( length, end - begin );
		
		const size_t pad_length = length - (end - begin);
		
		plus::var_string result;
		
		result.reserve( length );
		
		result.assign( begin, end );
		
		result.append( pad_length, ' ' );
		
		return result;
	}
	
	static inline plus::string left_padded( const plus::string& word, unsigned length )
	{
		return left_padded( &*word.begin(), &*word.end(), length );
	}
	
	static inline plus::string right_padded( const plus::string& word, unsigned length )
	{
		return right_padded( &*word.begin(), &*word.end(), length );
	}
	
	static plus::string report_process( const plus::string& pid_name )
	{
		n::owned< p7::fd_t > proc_pid = p7::openat( g_proc, pid_name, p7::o_rdonly | p7::o_directory );
		
		char buffer[ 4096 ];
		
		const char* begin = buffer;
		const char* end   = buffer + p7::read( p7::openat( proc_pid, "stat", p7::o_rdonly ), buffer, 4096 );
		
		const char* close_paren = std::find( begin, end, ')' );
		
		ASSERT( close_paren != end );
		
		const char* p_stat = close_paren + 2;
		
		const char* space = std::find( p_stat, end, ' ' );
		
		plus::var_string stat_string( p_stat, space );
		
		const char* p_ppid = space + 1;
		
		space = std::find( p_ppid, end, ' ' );
		
		plus::string ppid_string( p_ppid, space );
		
		const char* p_pgid = space + 1;
		
		space = std::find( p_pgid, end, ' ' );
		
		plus::string pgid_string( p_pgid, space );
		
		const char* p_sid = space + 1;
		
		space = std::find( p_sid, end, ' ' );
		
		plus::string sid_string( p_sid, space );
		
		const char* p_termname = space + 1;
		
		space = std::find( p_termname, end, ' ' );
		
		plus::var_string term_string( p_termname, space );
		
		const char* p_tpgid = space + 1;
		
		space = std::find( p_tpgid, end, ' ' );
		
		plus::string tpgid_string( p_tpgid, space );
		
		pid_t pid = iota::parse_unsigned_decimal( pid_name.c_str() );
		
		pid_t ppid  = iota::parse_unsigned_decimal( p_ppid  );
		pid_t pgid  = iota::parse_unsigned_decimal( p_pgid  );
		pid_t sid   = iota::parse_unsigned_decimal( p_sid   );
		pid_t tpgid = iota::parse_unsigned_decimal( p_tpgid );
		
		if ( term_string.length() == STRLEN( "/gui/port/12345678/tty" ) )
		{
			const char* address = term_string.data()
			                    + STRLEN( "/gui/port/" );
			
			term_string.assign( address, STRLEN( "12345678" ) );
		}
		else if ( term_string.length() > STRLEN( "/dev/" ) )
		{
			term_string.erase( 0, STRLEN( "/dev/" ) );
		}
		
		if ( pid == sid )
		{
			stat_string += 's';
		}
		
		if ( pgid == tpgid )
		{
			stat_string += '+';
		}
		
		plus::var_string report;
		
		report += left_padded( pid_name, 5 );
		
		report += " ";
		
		report += right_padded( term_string, 8 );
		
		report += " ";
		
		report += right_padded( stat_string, 4 );
		
		report += "  ";
		
		report += left_padded( ppid_string, 5 );
		
		report += "  ";
		
		report += left_padded( pgid_string, 5 );
		
		report += "  ";
		
		report += left_padded( sid_string, 5 );
		
		report += "  ";
		
		char* cmdline_end = buffer + p7::read( p7::openat( proc_pid, "cmdline", p7::o_rdonly ), buffer, 4096 );
		
		if ( cmdline_end > buffer )
		{
			std::replace( buffer, cmdline_end - 1, '\0', ' ' );  // replace NUL with space except last
			
			report.append( buffer, cmdline_end - 1 );
		}
		
		if ( !globally_wide  &&  report.size() > 80 )
		{
			report.resize( 80 );
		}
		
		report += "\n";
		
		return report;
	}
	
	static plus::string ps()
	{
		plus::var_string output = "  PID TERM     STAT   PPID   PGID    SID  COMMAND\n";
		
		DIR* iter = opendir( "/proc" );
		
		while ( const dirent* ent = readdir( iter ) )
		{
			if ( pid_t pid = iota::parse_unsigned_decimal( ent->d_name ) )
			{
				// A process could exit while we're examining it
				try
				{
					output += report_process( ent->d_name );
				}
				catch ( ... )
				{
				}
			}
		}
		
		closedir( iter );
		
		return output;
	}
	
	int Main( int argc, char** argv )
	{
		bool monitor = false;
		
		const char* sleep_arg = NULL;
		
		o::bind_option_to_variable( "--monitor", monitor );
		
		o::bind_option_to_variable( "--wide", globally_wide );
		
		o::bind_option_to_variable( "--sleep", sleep_arg );
		
		o::get_options( argc, argv );
		
		float min_sleep = 1.0;
		
		if ( sleep_arg )
		{
			monitor = true;
			
			min_sleep = gear::parse_float( sleep_arg );
		}
		
		const struct timespec minimum = timespec_from_seconds( min_sleep );
		
		if ( !monitor )
		{
			p7::write( p7::stdout_fileno, ps() );
			
			return 0;
		}
		
		plus::string output;
		plus::string previous;
		
	again:
		
		output = ps();
		
		if ( output != previous )
		{
			if ( monitor )
			{
				p7::ftruncate( p7::stdout_fileno, output.size() );
				
				p7::pwrite( p7::stdout_fileno, output, 0 );
				
				using std::swap;
				
				swap( output, previous );
			}
		}
		
		if ( monitor )
		{
			int dozed = doze( &minimum, NULL, NULL );  // min, max, remaining
			
			goto again;
		}
		
		return 0;
	}
	
}

