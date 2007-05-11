/*	========
 *	jtest.cc
 *	========
 */

// Standard C
#include "errno.h"
#include "stdlib.h"

// Standard C/C++
#include <cctype>

// Standard C++
#include <algorithm>
#include <functional>
#include <vector>

// POSIX
#include "fcntl.h"
#include "sys/wait.h"
#include "unistd.h"

// Nucleus
#include "Nucleus/Convert.h"

// POSeven
#include "POSeven/FileDescriptor.hh"

// Io
#include "Io/TextInput.hh"

// Orion
#include "Orion/Main.hh"


namespace NN = Nucleus;
namespace P7 = POSeven;
namespace O = Orion;


static bool PathnameMeansStdIn( const char* pathname )
{
	return    pathname[0] == '-'
	       && pathname[1] == '\0';
}

static int exit_from_wait( int stat )
{
	int result = WIFEXITED( stat )   ? WEXITSTATUS( stat )
	           : WIFSIGNALED( stat ) ? WTERMSIG( stat ) + 128
	           :                       -1;
	
	return result;
}

enum IoOperator
{
	kNoOp = -1,
	
	kIOMask = 0x01,
		kInput  = 0,
		kOutput = 1,
	
	kFormatMask = 0x02,
		kOneLine = 0,
		kHereDoc = 2,
	
	kInputLine       = kInput  | kOneLine,  // 0  >=
	kInputHere       = kInput  | kHereDoc,  // 1  >>
	kMatchOutputLine = kOutput | kOneLine,  // 2  <=
	kMatchOutputHere = kOutput | kHereDoc,  // 3  <<
	
	kClosed = 4                             // 4  --
};

static IoOperator ReadOp( const char* s )
{
	bool input = false;
	bool here = false;
	
	switch ( s[0] )
	{
		case '-':
			return s[1] == '-' ? kClosed : kNoOp;
		
		case '<':
			input = true;
			// fall through
		
		case '>':
			if      ( s[1] == s[0] )  here = true;
			else if ( s[1] != '='  )  return kNoOp;
			
			return IoOperator( (input ? kInput : kOutput)  |  (here ? kHereDoc : kOneLine) );
		
		default:
			return kNoOp;
	}
}

struct Redirection
{
	int          fd;
	IoOperator   op;
	std::string  data;
	
	Redirection( int fd, IoOperator op, const std::string& data ) : fd( fd ), op( op ), data( data )  {}
};


static Redirection GetRedirectionFromLine( const std::string& line, Io::TextInputAdapter< P7::FileDescriptor >& input )
{
	std::size_t end_of_fd = line.find_first_not_of( "0123456789" );
	
	int fd_to_redirect = std::atoi( line.substr( 0, end_of_fd - 0 ).c_str() );
	
	std::size_t start_of_op = line.find_first_not_of( " \t", end_of_fd );
	
	std::size_t end_of_op = start_of_op + 2;
	
	if ( line.size() < end_of_op )
	{
		std::fprintf( stderr, "Missing operator in line: %s\n", line.c_str() );
		O::ThrowExitStatus( 1 );
	}
	
	IoOperator op = ReadOp( line.c_str() + start_of_op );
	
	if ( op == kNoOp )
	{
		std::fprintf( stderr, "Unrecognized operator in line: %s\n", line.c_str() );
		O::ThrowExitStatus( 1 );
	}
	
	std::string param;
	
	if ( op != kClosed )
	{
		std::size_t start_of_param = line.find_first_not_of( " \t", end_of_op );
		
		char param0 = line[ start_of_param ];
		char delimiter = ' ';
		
		if ( param0 == '"'  ||  param0 == '\'' )
		{
			delimiter = param0;
			
			++start_of_param;
		}
		
		std::size_t end_of_param = line.find( delimiter, start_of_param );
		
		param = line.substr( start_of_param, end_of_param - start_of_param );
		
		if ( (op & kFormatMask) == kOneLine )
		{
			param += "\n";
		}
		else
		{
			std::string hereDoc;
			bool premature_EOF = false;
			
			while ( !( input.Ended() && (premature_EOF = true) ) )
			{
				std::string nextLine = input.Read();
				
				if ( nextLine == param )
				{
					break;
				}
				
				hereDoc += nextLine + "\n";
			}
			
			if ( premature_EOF )
			{
				std::fprintf( stderr, "Missing heredoc terminator '%s'\n", param.c_str() );
				
				O::ThrowExitStatus( 1 );
			}
			
			std::swap( param, hereDoc );
		}
		
		//op &= kIOMask;
		op = IoOperator( op & kIOMask );
	}
	
	return Redirection( fd_to_redirect, op, param );
}


static std::string PrefixLines( const std::string text, const char* prefix = "# " )
{
	std::string result;
	
	const char* p = text.c_str();
	
	while ( const char* q = std::strchr( p, '\n' ) )
	{
		result += prefix;
		
		result.append( p, ++q );
		
		p = q;
	}
	
	// *p == '\0' unless there's an unterminated trailing line
	
	return result;
}

static bool DiscrepantOutput( const Redirection& redir )
{
	if ( redir.op != kOutput )  return false;
	
	std::string actual_output;
	
	char data[ 4096 ];
	
	while ( int bytes_read = read( redir.fd, data, 4096 ) )
	{
		if ( bytes_read == -1 )
		{
			bytes_read = 0;
			std::perror( "Error reading captured output" );
			
			return true;
		}
		
		actual_output.append( data, bytes_read );
	}
	
	
	close( redir.fd );
	
	bool match = actual_output == redir.data;
	
	if ( !match )
	{
		std::fprintf( stdout, "# EXPECTED:\n%s"
		                      "# RECEIVED:\n%s", PrefixLines( redir.data    ).c_str(),
		                                         PrefixLines( actual_output ).c_str() );
	}
	
	return !match;
}


struct Pipe
{
	int itsFDs[2];
	
	Pipe()  {}
	
	Pipe( const int fds[2] )  { std::copy( fds, fds + 2, itsFDs ); }
};


class TestCase
{
	private:
		std::string itsCommand;
		std::string itsToDoReason;
		int itsExpectedExitStatus;
		std::vector< Redirection > itsRedirections;
		unsigned itsCountOfPipesNeeded;
		std::vector< Pipe > itsPipes;
		std::vector< int > itsWriteEnds;
	
	public:
		TestCase() : itsExpectedExitStatus(), itsCountOfPipesNeeded()  {}
		
		void SetCommand( const std::string& command )  { itsCommand = command; }
		const std::string& GetToDoReason() const  { return itsToDoReason; }
		void SetToDoReason( const std::string& reason )  { itsToDoReason = reason; }
		void SetExitStatus( int status )  { itsExpectedExitStatus = status; }
		void AddRedirection( const Redirection& redir );
		
		void Redirect( Redirection& redir );
		
		bool Run();
	
	private:
		struct Redirecting
		{
			TestCase& test;
			
			Redirecting( TestCase& test ) : test( test )  {}
			void operator()( Redirection& redir )  { test.Redirect( redir ); }
		};
		
		Redirecting Redirector()  { return Redirecting( *this ); }
		
		void CheckForCompleteness();
		
		void CreatePipes();
		
		void ClosePipeWriters();
		
		bool DoesOutputMatch() const;
};


void TestCase::AddRedirection( const Redirection& redir )
{
	itsRedirections.push_back( redir );
	
	if ( redir.op == kOutput )
	{
		++itsCountOfPipesNeeded;
	}
}

void TestCase::CheckForCompleteness()
{
	if ( itsCommand.empty() )
	{
		std::fprintf( stderr, "Command missing\n" );
		
		O::ThrowExitStatus( 1 );
	}
}


void TestCase::CreatePipes()
{
	unsigned pipes = itsCountOfPipesNeeded;
	
	while ( pipes-- )
	{
		int fds[2];
		pipe( fds );
		itsPipes.push_back( fds );
	}
}

void TestCase::Redirect( Redirection& redir )
{
	int pipe_fds[2];
	int* fds = pipe_fds;
	
	switch ( redir.op )
	{
		case kClosed:
			close( redir.fd );
			break;
		
		case kInputLine:
			pipe( fds );
			write( fds[1], redir.data.data(), redir.data.size() );
			close( fds[1] );
			dup2( fds[0], redir.fd );
			close( fds[0] );
			break;
		
		case kMatchOutputLine:
			fds = itsPipes.back().itsFDs;
			dup2( fds[1], redir.fd );
			close( fds[1] );
			redir.fd = fds[0];
			close( fds[0] );
			itsWriteEnds.push_back( fds[1] );
			itsPipes.pop_back();
			break;
		
		default:
			std::fprintf( stderr, "Error in redirection\n" );
			break;
	}
}

void TestCase::ClosePipeWriters()
{
	std::for_each( itsWriteEnds.begin(),
		           itsWriteEnds.end(),
		           std::ptr_fun( close ) );
	
	itsWriteEnds.clear();
}

bool TestCase::DoesOutputMatch() const
{
	std::vector< Redirection >::const_iterator it = std::find_if( itsRedirections.begin(),
	                                                              itsRedirections.end(),
	                                                              std::ptr_fun( DiscrepantOutput ) );
	
	bool output_matches = it == itsRedirections.end();
	
	return output_matches;
}


bool TestCase::Run()
{
	CheckForCompleteness();
	
	CreatePipes();
	
	pid_t pid = vfork();
	
	if ( pid == 0 )
	{
		std::for_each( itsRedirections.begin(),
		               itsRedirections.end(),
		               Redirector() );
		
		const char* argv[] = { "sh", "-c", "", NULL };
		
		argv[2] = itsCommand.c_str();
		
		execv( "/bin/sh", (char**) argv );
		
		_exit( 127 );
	}
	
	ClosePipeWriters();
	
	int wait_status = -1;
	pid_t resultpid = waitpid( pid, &wait_status, 0 );
	
	bool output_matches = DoesOutputMatch();
	bool status_matches = exit_from_wait( wait_status ) == itsExpectedExitStatus;
	
	bool test_ok = status_matches && output_matches;
	
	return test_ok;
}


static void RunTest( TestCase& test )
{
	static unsigned gLastNumber = 0;
	
	bool test_ok = test.Run();
	
	std::string result = test_ok ? "ok" : "not ok";
	
	result += " " + NN::Convert< std::string >( ++gLastNumber );
	
	const std::string& reason = test.GetToDoReason();
	
	if ( !reason.empty() )
	{
		result += " # TODO " + reason;
	}
	
	result += "\n";
	
	(void) write( STDOUT_FILENO, result.data(), result.size() );
}

int O::Main( int argc, const char *const argv[] )
{
	const char* jtest = argv[0];
	
	int fd = 0;  // Default to stdin
	
	if ( argc > 1  &&  !PathnameMeansStdIn( argv[1] ) )
	{
		fd = open( argv[1], O_RDONLY );
		
		if ( fd == -1 )
		{
			std::fprintf( stderr, "%s: %s: %s\n", jtest, argv[1], std::strerror( errno ) );
			
			return 1;
		}
		
		int controlled = fcntl( fd, F_SETFD, FD_CLOEXEC );
	}
	
	Io::TextInputAdapter< P7::FileDescriptor > input = P7::FileDescriptor( fd );
	
	std::vector< TestCase > battery;
	TestCase test;
	
	while ( !input.Ended() )
	{
		std::string line = input.Read();
		
		if ( line.empty() )  continue;
		
		if ( line[0] == '#' )
		{
			// comment
			continue;
		}
		
		if ( line[0] == '$' )
		{
			std::string command = line.substr( line.find_first_not_of( " \t", 1 ), line.npos );
			
			test.SetCommand( command );
			
			continue;
		}
		
		if ( line[0] == '?' )
		{
			int exit_status = std::atoi( line.substr( line.find_first_not_of( " \t", 1 ), line.npos ).c_str() );
			test.SetExitStatus( exit_status );
			continue;
		}
		
		if ( std::isdigit( line[0] ) )
		{
			test.AddRedirection( GetRedirectionFromLine( line, input ) );
			
			continue;
		}
		
		if ( line[0] == '%' )
		{
			battery.push_back( test );
			test = TestCase();
			continue;
		}
		
		if ( line.substr( 0, 4 ) == "TODO" )
		{
			test.SetToDoReason( line.substr( 5, line.npos ) );
			continue;
		}
		
		std::fprintf( stderr, "Unprocessed line: %s\n", line.c_str() );
	}
	
	battery.push_back( test );
	
	std::string header = "1.." + NN::Convert< std::string >( battery.size() ) + "\n";
	
	(void) write( STDOUT_FILENO, header.data(), header.size() );
	
	std::for_each( battery.begin(), battery.end(), std::ptr_fun( RunTest ) );
	
	return 0;
}

