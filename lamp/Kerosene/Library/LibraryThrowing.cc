/*	==================
 *	LibraryThrowing.cc
 *	==================
 */

// Mac OS Universal Interfaces
#include <LowMem.h>

// Standard C
#include "errno.h"
#include <stdarg.h>
#include "stdlib.h"

// Standard C++
#include <string>
#include <vector>

// POSIX
#include "dirent.h"
#include "fcntl.h"
#include "sys/stat.h"
#include "unistd.h"

// Iota
#include "iota/environ.hh"

// ShellShock
#include "ShellShock/VarArray.hh"


//

	static std::string LookupPath( const char* filename )
	{
		const char* pathVar = getenv( "PATH" );
		
		if ( pathVar == NULL )
		{
			return "";
		}
		
		const char* pathEnd = pathVar + std::strlen( pathVar );
		
		while ( pathVar < pathEnd )
		{
			const char* separator = std::strchr( pathVar, ':' );
			
			if ( separator == NULL )
			{
				separator = pathEnd;
			}
			
			// Watch out for empty path elements (e.g. "/bin:/sbin:" -- last is empty)
			if ( separator != pathVar )
			{
				std::string dir( pathVar, separator - pathVar );
				
				std::string tryPath = dir + "/" + filename;
				
				struct ::stat sb;
				int status = stat( tryPath.c_str(), &sb );
				
				if ( status == 0  &&  sb.st_mode & S_IXUSR )
				{
					return tryPath;
				}
			}
			
			// If we're at the end, then this sets pathVar > pathEnd
			pathVar = separator + 1;
		}
		
		return "";
	}
	
	int execl( const char* path, const char* arg0, ... )
	{
		va_list va;
		std::vector< const char* > args;
		
		args.push_back( arg0 );
		
		va_start( va, arg0 );
		
		while ( const char* arg = va_arg( va, const char * ) )
		{
			args.push_back( arg );
		}
		
		va_end( va );
		
		args.push_back( NULL );
		
		const char* const* argv = &args[0];
		
		return execv( path, argv );
	}
	
	int execle( const char* path, const char* arg0, ... )
	{
		va_list va;
		std::vector< const char* > args;
		
		args.push_back( arg0 );
		
		va_start( va, arg0 );
		
		while ( const char* arg = va_arg( va, const char * ) )
		{
			args.push_back( arg );
		}
		
		args.push_back( NULL );
		
		const char* const* envp = va_arg( va, const char* const* );
		
		va_end( va );
		
		const char* const* argv = &args[0];
		
		return execve( path, argv, envp );
	}
	
	int execlp( const char* file, const char* arg0, ... )
	{
		
		va_list va;
		std::vector< const char* > args;
		
		args.push_back( arg0 );
		
		va_start( va, arg0 );
		
		while ( const char* arg = va_arg( va, const char * ) )
		{
			args.push_back( arg );
		}
		
		va_end( va );
		
		args.push_back( NULL );
		
		const char* const* argv = &args[0];
		
		return execvp( file, argv );
	}
	
	int execvp( const char* file, const char* const argv[] )
	{
		std::string path;
		
		if ( std::strchr( file, '/' ) == NULL )
		{
			path = LookupPath( file );
			
			if ( path == "" )
			{
				errno = ENOENT;
				
				return -1;
			}
			
			file = path.c_str();
		}
		
		return execv( file, argv );
	}
	
	
	iota::environ_t environ = reinterpret_cast< iota::environ_t* >( LMGetToolScratch() )[1];
	
	
	class Environ
	{
		private:
			std::auto_ptr< ShellShock::VarArray > itsEnvironStorage;
			
			std::string itsLastEnv;
		
		public:
			Environ();
			
			~Environ();
			
			void UpdateEnvironValue();
			
			char* GetEnv( const char* name );
			void SetEnv( const char* name, const char* value, bool overwrite );
			void PutEnv( const char* string );
			void UnsetEnv( const char* name );
			void ClearEnv();
	};
	
	
	namespace Sh = ShellShock;
	
	Environ::Environ() : itsEnvironStorage( environ ? new Sh::VarArray( environ )
	                                                : new Sh::VarArray(         ) )
	{
		UpdateEnvironValue();
	}
	
	Environ::~Environ()
	{
	}
	
	void Environ::UpdateEnvironValue()
	{
		environ = itsEnvironStorage->GetPointer();
	}
	
	char* Environ::GetEnv( const char* name )
	{
		char* result = NULL;
		
		Sh::SVector::const_iterator it = itsEnvironStorage->Find( name );
		
		const char* var = *it;
		
		const char* end = Sh::EndOfVarName( var );
		
		// Did we find the right environment variable?
		if ( end != NULL  &&  *end == '='  &&  Sh::VarMatchesName( var, end, name ) )
		{
			itsLastEnv = var;
			itsLastEnv += "\0";  // make sure we have a trailing null
			
			std::size_t offset = end + 1 - var;
			
			result = &itsLastEnv[ offset ];
		}
		
		return result;
	}
	
	void Environ::SetEnv( const char* name, const char* value, bool overwrite )
	{
		Sh::SVector::iterator it = itsEnvironStorage->Find( name );
		
		const char* var = *it;
		
		// Did we find the right environment variable?
		bool match = Sh::VarMatchesName( var, Sh::EndOfVarName( var ), name );
		
		// If it doesn't match, we insert (otherwise, we possibly overwrite)
		bool inserting = !match;
		
		if ( inserting )
		{
			itsEnvironStorage->Insert( it, Sh::MakeVar( name, value ) );
			
			UpdateEnvironValue();
		}
		else if ( overwrite )
		{
			itsEnvironStorage->Overwrite( it, Sh::MakeVar( name, value ) );
		}
	}
	
	void Environ::PutEnv( const char* string )
	{
		std::string name = string;
		name.resize( name.find( '=' ) );
		
		Sh::SVector::iterator it = itsEnvironStorage->Find( name.c_str() );
		
		const char* var = *it;
		
		// Did we find the right environment variable?
		bool match = Sh::VarMatchesName( var, Sh::EndOfVarName( var ), name.c_str() );
		
		// If it doesn't match, we insert (otherwise, we possibly overwrite)
		bool inserting = !match;
		
		if ( inserting )
		{
			itsEnvironStorage->Insert( it, string );
			
			UpdateEnvironValue();
		}
		else
		{
			itsEnvironStorage->Overwrite( it, string );
		}
	}
	
	void Environ::UnsetEnv( const char* name )
	{
		Sh::SVector::iterator it = itsEnvironStorage->Find( name );
		
		const char* var = *it;
		
		// Did we find the right environment variable?
		bool match = Sh::VarMatchesName( var, Sh::EndOfVarName( var ), name );
		
		if ( match )
		{
			itsEnvironStorage->Remove( it );
		}
	}
	
	void Environ::ClearEnv()
	{
		itsEnvironStorage->Clear();
		
		environ = NULL;
	}
	
	static Environ gEnviron;
	
	
	char* getenv( const char* name )
	{
		return gEnviron.GetEnv( name );
	}
	
	int setenv( const char* name, const char* value, int overwrite )
	{
		gEnviron.SetEnv( name, value, overwrite );
		
		return 0;
	}
	
	int putenv( const char* string )
	{
		gEnviron.PutEnv( string );
		
		return 0;
	}
	
	void unsetenv( const char* name )
	{
		gEnviron.UnsetEnv( name );
	}
	
	extern "C" int clearenv();
	
	int clearenv()
	{
		gEnviron.ClearEnv();
		
		return 0;
	}
	
	
	DIR* opendir( const char* pathname )
	{
		DIR* result = NULL;
		
		try
		{
			DIR* dir = new DIR;
			
			int fd = open( pathname, O_RDONLY, 0 );
			
			if ( fd == -1 )
			{
				delete dir;
			}
			else
			{
				dir->fd = fd;
				
				result = dir;
			}
		}
		catch ( ... )
		{
		}
		
		return result;
	}
	
	struct dirent* readdir( DIR* dir )
	{
		static dirent entry;
		
		int got = getdents( dirfd( dir ), &entry, sizeof (dirent) );
		
		if ( got <= 0 )
		{
			return NULL;
		}
		
		return &entry;
	}
	
	int closedir( DIR* dir )
	{
		int fd = dirfd( dir );
		
		delete dir;
		
		return close( fd );
	}
	
	void rewinddir( DIR* dir )
	{
		(void) lseek( dirfd( dir ), 0, SEEK_SET );
	}
	
	long telldir( DIR* dir )
	{
		return lseek( dirfd( dir ), 0, SEEK_CUR );
	}
	
	void seekdir( DIR* dir, long offset )
	{
		(void) lseek( dirfd( dir ), offset, SEEK_SET );
	}
	
	int dirfd( DIR* dir )
	{
		return dir->fd;
	}
	
//

