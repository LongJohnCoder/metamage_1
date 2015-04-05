/*	=========
 *	tlsrvr.cc
 *	=========
 */

// Standard C/C++
#include <cctype>

// Standard C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// gear
#include "gear/inscribe_decimal.hh"

// plus
#include "plus/mac_utf8.hh"
#include "plus/var_string.hh"

// poseven
#include "poseven/functions/perror.hh"

// Nitrogen
#include "Mac/Toolbox/Types/OSStatus.hh"

// OSErrno
#include "OSErrno/OSErrno.hh"

// Orion
#include "Orion/get_options.hh"
#include "Orion/Main.hh"

// tlsrvr
#include "ToolServer.hh"
#include "RunToolServer.hh"


namespace tool
{
	
	namespace p7 = poseven;
	namespace o = orion;
	
	
	static plus::string QuoteForMPW( const plus::string& str )
	{
		const char* p = str.c_str();
		const char* q = p;
		
		bool needsQuoting = false;
		
		plus::var_string result = "'";
		
		while ( *p != '\0' )
		{
			while ( *q != '\0'  &&  *q != '\'' )
			{
				needsQuoting = needsQuoting || !std::isalnum( *q );
				++q;
			}
			
			result.append( p, q );
			
			if ( *q != '\0' )
			{
				needsQuoting = true;
				result += sEscapedQuote;
				++q;
			}
			
			p = q;
		}
		
		if ( !needsQuoting )
		{
			return str;
		}
		
		result += "'";
		
		return result;
	}
	
	
	static plus::string MakeCommand( char const *const *begin, char const *const *end, bool needToEscape )
	{
		plus::var_string command;
		
		for ( char const *const *it = begin;  it < end;  ++it )
		{
			plus::string word = plus::string( *it,
			                                  strlen( *it ),
			                                  plus::delete_never );
			
			word = plus::mac_from_utf8( word );
			
			command += needToEscape ? QuoteForMPW( word ) : word;
			
			command += " ";
		}
		
		if ( !command.empty() )
		{
			command.resize( command.size() - 1 );
		}
		
		return command;
	}
	
	int Main( int argc, char** argv )
	{
		bool escapeForMPW = false;
		bool switchLayers = false;
		
		o::bind_option_to_variable( "--escape", escapeForMPW );
		o::bind_option_to_variable( "--switch", switchLayers );
		
		o::get_options( argc, argv );
		
		char const *const *free_args = o::free_arguments();
		
		if ( const char* frontmost = getenv( "TLSRVR_FRONTMOST" ) )
		{
			if ( strcmp( frontmost, "always" ) == 0 )
			{
				switchLayers = true;
			}
			else if ( strcmp( frontmost, "never" ) == 0 )
			{
				// do nothing
			}
			else
			{
				fprintf( stderr, "TLSRVR_FRONTMOST must be always|never, not %s\n", frontmost );
				
				return 2;
			}
		}
		
		plus::string command = MakeCommand( free_args,
		                                    free_args + o::free_argument_count(),
		                                    escapeForMPW );
		
		try
		{
			return RunCommandInToolServer( command, switchLayers );
		}
		catch ( const Mac::OSStatus& err )
		{
			plus::var_string status = "OSStatus ";
			
			status += gear::inscribe_decimal( err );
			
			p7::perror( "tlsrvr", status, 0 );
			
			p7::throw_errno( OSErrno::ErrnoFromOSStatus( err ) );
		}
		
		// Not reached
		return 0;
	}

}
