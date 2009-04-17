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
#include <set>
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
	
	
	iota::environ_t environ = NULL;
	
	
	inline iota::environ_t GetEnvironFromKernel()
	{
		return reinterpret_cast< iota::environ_t* >( LMGetToolScratch() )[1];
	}
	
	inline char* copy_string( const char* s )
	{
		std::size_t len = std::strlen( s );
		
		char* result = new char[ len + 1 ];
		
		std::copy( s, s + len + 1, result );
		
		return result;
	}
	
	inline char* copy_string( const std::string& s )
	{
		char* result = new char[ s.size() + 1 ];
		
		std::copy( s.c_str(), s.c_str() + s.size() + 1, result );
		
		return result;
	}
	
	static void DeleteVars( std::vector< char* >& result )
	{
		for ( int i = result.size() - 1;  i >= 0;  --i )
		{
			delete [] result[ i ];
		}
	}
	
	static void CopyVars( char const *const *vars, std::vector< char* >& result  )
	{
		try
		{
			if ( vars != NULL )
			{
				while ( *vars )
				{
					// This ensures push_back() won't (fail to) allocate memory
					result.reserve( result.size() + 1 );
					
					result.push_back( copy_string( *vars++ ) );
				}
			}
			
			result.push_back( NULL );
		}
		catch ( ... )
		{
			DeleteVars( result );
			
			throw;
		}
	}
	
	
	namespace Sh = ShellShock;
	
	static std::vector< char* >::iterator FindVar( std::vector< char* >& vars, const char* name )
	{
		return std::lower_bound( vars.begin(),
		                         vars.end() - 1,
		                         name,
		                         std::ptr_fun( Sh::CompareStrings ) );
	}
	
	
	// This exercise is necessary because we need to pass the string with
	// different constness depending on whether we're setting or putting.
	// With setenv(), we construct a new std::string on which we call c_str(),
	// which returns const char*.  With putenv(), we're using the actual string,
	// and storing it as char*.  Type-safety for the win.
	
	template < bool putting > struct overwrite_traits;
	
	template <> struct overwrite_traits< false >
	{
		static const bool user_owned = false;
		
		typedef const char* param_type;
		
		static char* new_storage( param_type, std::size_t length )
		{
			return new char[ length + 1 ];
		}
	};
	
	template <> struct overwrite_traits< true >
	{
		static const bool user_owned = true;
		
		typedef char* param_type;
		
		static char* new_storage( param_type string, std::size_t )
		{
			return string;
		}
	};
	
	
	class Environ
	{
		private:
			std::vector< char* >     itsVars;
			std::set< const char* >  itsUserOwnedVars;
			
		private:
			// Non-copyable
			Environ           ( const Environ& );
			Environ& operator=( const Environ& );
		
		public:
			Environ();
			
			~Environ();
			
			void UpdateEnvironValue();
			void Preallocate();
			
			template < bool putting >
			void OverwriteVar( std::vector< char* >::iterator                    it,
	                           typename overwrite_traits< putting >::param_type  string,
	                           std::size_t                                       new_len );
			
			void RemoveUserOwnedVars();
			
			char* GetEnv( const char* name );
			void SetEnv( const char* name, const char* value, bool overwrite );
			void PutEnv( char* string );
			void UnsetEnv( const char* name );
			void ClearEnv();
	};
	
	
	Environ::Environ()
	{
		CopyVars( GetEnvironFromKernel(), itsVars );
		
		UpdateEnvironValue();
	}
	
	Environ::~Environ()
	{
		ClearEnv();
	}
	
	void Environ::UpdateEnvironValue()
	{
		environ = &itsVars.front();
	}
	
	void Environ::Preallocate()
	{
		// We reserve an extra slot so we can later insert without allocating memory, which
		// (a) could fail and throw bad_alloc, or
		// (b) could succeed and invalidate iterators.
		
		itsVars.reserve( itsVars.size() + 1 );
		
		UpdateEnvironValue();
	}
	
	template < bool putting >
	void Environ::OverwriteVar( std::vector< char* >::iterator                    it,
	                            typename overwrite_traits< putting >::param_type  string,
	                            std::size_t                                       new_len )
	{
		typedef overwrite_traits< putting > traits;
		
		// true for putenv(), false for setenv(), known at compile time.
		const bool new_is_user_owned = traits::user_owned;
		
		char* var = *it;
		
		std::size_t old_len = std::strlen( var );
		
		std::set< const char* >::iterator user_ownership = itsUserOwnedVars.find( var );
		
		// true for putenv(), false for setenv(), known at runtime.
		bool old_is_user_owned = user_ownership != itsUserOwnedVars.end();
		
		// If neither var string is user-owned and the lengths are the same,
		// it's a straight copy -- no memory management is required.
		// User-owned var strings don't get allocated or deallocated here,
		// but instead we have to mark them so we don't delete them later.
		
		if ( old_is_user_owned  ||  new_is_user_owned  ||  new_len != old_len )
		{
			if ( new_is_user_owned )
			{
				itsUserOwnedVars.insert( string );  // may throw
			}
			
			*it = traits::new_storage( string, new_len );
			
			if ( old_is_user_owned )
			{
				itsUserOwnedVars.erase( user_ownership );
			}
			else
			{
				delete [] var;
			}
			
			var = *it;
		}
		
		if ( !new_is_user_owned )
		{
			std::copy( string, string + new_len + 1, var );
		}
	}
	
	void Environ::RemoveUserOwnedVars()
	{
		// Here we zero out user-owned var string storage.  This is a convenience
		// that allows us to subsequently call DeleteVars() safely without
		// giving it a dependency on the user ownership structure.
		
		for ( std::vector< char* >::iterator it = itsVars.begin();  it != itsVars.end();  ++it )
		{
			std::set< const char* >::iterator user_ownership = itsUserOwnedVars.find( *it );
			
			if ( user_ownership != itsUserOwnedVars.end() )
			{
				//itsUserOwnedVars.erase( user_ownership );
				
				*it = NULL;
			}
		}
		
		itsUserOwnedVars.clear();
	}
	
	char* Environ::GetEnv( const char* name )
	{
		std::vector< char* >::iterator it = FindVar( itsVars, name );
		
		char* var = *it;
		
		const char* end = Sh::EndOfVarName( var );
		
		// Did we find the right environment variable?
		if ( end != NULL  &&  *end == '='  &&  Sh::VarMatchesName( var, end, name ) )
		{
			return var + (end - var) + 1;
		}
		
		return NULL;
	}
	
	void Environ::SetEnv( const char* name, const char* value, bool overwrite )
	{
		Preallocate();  // make insertion safe
		
		std::vector< char* >::iterator it = FindVar( itsVars, name );
		
		const char* var = *it;
		
		// Did we find the right environment variable?
		bool match = Sh::VarMatchesName( var, Sh::EndOfVarName( var ), name );
		
		// If it doesn't match, we insert (otherwise, we possibly overwrite)
		bool inserting = !match;
		
		if ( inserting )
		{
			// copy_string() may throw, but insert() will not
			itsVars.insert( it, copy_string( Sh::MakeVar( name, value ) ) );
		}
		else if ( overwrite )
		{
			std::string new_var = Sh::MakeVar( name, value );
			
			OverwriteVar< false >( it, new_var.c_str(), new_var.length() );
		}
	}
	
	void Environ::PutEnv( char* string )
	{
		std::string name = string;
		
		std::size_t length = name.length();
		
		name.resize( name.find( '=' ) );
		
		Preallocate();  // make insertion safe
		
		std::vector< char* >::iterator it = FindVar( itsVars, name.c_str() );
		
		const char* var = *it;
		
		// Did we find the right environment variable?
		bool match = Sh::VarMatchesName( var, Sh::EndOfVarName( var ), name.c_str() );
		
		// If it doesn't match, we insert (otherwise, we possibly overwrite)
		bool inserting = !match;
		
		if ( inserting )
		{
			itsUserOwnedVars.insert( string );  // may throw
			
			itsVars.insert( it, string );  // memory already reserved
		}
		else
		{
			OverwriteVar< true >( it, string, length );
		}
	}
	
	void Environ::UnsetEnv( const char* name )
	{
		std::vector< char* >::iterator it = FindVar( itsVars, name );
		
		const char* var = *it;
		
		// Did we find the right environment variable?
		bool match = Sh::VarMatchesName( var, Sh::EndOfVarName( var ), name );
		
		
		if ( match )
		{
			std::set< const char* >::iterator user_ownership = itsUserOwnedVars.find( var );
			
			bool user_owned = user_ownership != itsUserOwnedVars.end();
			
			if ( user_owned )
			{
				itsUserOwnedVars.erase( user_ownership );
			}
			
			itsVars.erase( it );
			
			delete [] var;
		}
	}
	
	void Environ::ClearEnv()
	{
		// Zero out user-owned memory so we don't try to delete it.
		RemoveUserOwnedVars();
		
		DeleteVars( itsVars );
		
		itsVars.clear();
		
		itsVars.resize( 1, NULL );
		
		environ = NULL;
	}
	
	
	static Environ* gEnvironPtr;
	
	extern "C" const void* InitializeEnviron();
	
	const void* InitializeEnviron()
	{
		try
		{
			static Environ gEnviron;
			
			gEnvironPtr = &gEnviron;
		}
		catch ( ... )
		{
		}
		
		return gEnvironPtr;  // NULL if bad_alloc
	}
	
	
	char* getenv( const char* name )
	{
		return gEnvironPtr->GetEnv( name );
	}
	
	int setenv( const char* name, const char* value, int overwrite )
	{
		gEnvironPtr->SetEnv( name, value, overwrite );
		
		return 0;
	}
	
	int putenv( char* string )
	{
		gEnvironPtr->PutEnv( string );
		
		return 0;
	}
	
	void unsetenv( const char* name )
	{
		gEnvironPtr->UnsetEnv( name );
	}
	
	extern "C" int clearenv();
	
	int clearenv()
	{
		gEnvironPtr->ClearEnv();
		
		return 0;
	}
	
	
	DIR* fdopendir( int fd )
	{
		DIR* result = NULL;
		
		if ( fd < 0 )
		{
			errno = EBADF;
			
			return result;
		}
		
		try
		{
			result = new DIR;
			
			result->fd = fd;
		}
		catch ( ... )
		{
			errno = ENOMEM;
		}
		
		return result;
	}
	
	DIR* opendir( const char* pathname )
	{
		DIR* result = NULL;
		
		try
		{
			DIR* dir = new DIR;
			
			int fd = open( pathname, O_RDONLY | O_DIRECTORY );
			
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
			errno = ENOMEM;
		}
		
		return result;
	}
	
	struct dirent* readdir( DIR* dir )
	{
		struct dirent *const entry = &dir->entry;
		
		int got = getdents( dirfd( dir ), entry, sizeof (dirent) );
		
		if ( got <= 0 )
		{
			return NULL;
		}
		
		return entry;
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

