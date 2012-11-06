/*	================
 *	environ_store.cc
 *	================
 */

#include "environ_store.hh"

// Standard C/C++
#include <cstring>

// Debug
#include "debug/assert.hh"


char** environ = NULL;


namespace std
{
	
	struct iterator_traits< char** >
	{
		typedef ptrdiff_t difference_type ;
		typedef char* value_type ;
		typedef char** pointer ;
		typedef char*& reference ;
		typedef random_access_iterator_tag iterator_category ;
	};
}

namespace kerosene
{
	
	static inline char* copy_string( const char* s )
	{
		const std::size_t len = std::strlen( s );
		
		char *const result = new char[ len + 1 ];
		
		std::copy( s, s + len + 1, result );
		
		return result;
	}
	
	static void delete_vars( std::vector< char* >& result )
	{
		for ( int i = result.size() - 1;  i >= 0;  --i )
		{
			delete [] result[ i ];
		}
	}
	
	static inline void copy_vars( char const *const *vars, std::vector< char* >& result  )
	{
		try
		{
			if ( vars != NULL )
			{
				while ( *vars )
				{
					// This ensures push_back() won't (fail to) allocate memory
					result.push_back( NULL );
					result.pop_back();
					
					result.push_back( copy_string( *vars++ ) );
				}
			}
			
			result.push_back( NULL );
		}
		catch ( ... )
		{
			delete_vars( result );
			
			throw;
		}
	}
	
	
	// var is either NULL or of the form "name=oldvalue"
	// name may be either "name" or "name=newvalue"
	// Returns pointer to "oldvalue" on match or NULL otherwise.
	
	static char* var_match( char* var, const char* name )
	{
		if ( var )
		{
			for ( ;  ;  ++var, ++name )
			{
				const char c = *name != '\0' ? *name : '=';
				
				if ( *var != c )
				{
					break;
				}
				
				if ( c == '=' )
				{
					return var + 1;
				}
			}
		}
		
		return NULL;
	}
	
	static inline const char* var_match( const char* var, const char* name )
	{
		return var_match( const_cast< char* >( var ), name );
	}
	
	static bool var_less( const char* var, const char* name )
	{
		for ( ;  ;  ++var, ++name )
		{
			const char c = *name != '\0' ? *name : '=';
			
			if ( c == '=' )
			{
				// End of name.  var is longer or equal length, not less.
				return false;
			}
			
			if ( *var != c )
			{
				return *var < c  ||  *var == '=';
			}
		}
	}
	
	static std::vector< char* >::iterator find_var( std::vector< char* >& vars, const char* name )
	{
		return std::lower_bound( vars.begin(),
		                         vars.end() - 1,
		                         name,
		                         std::ptr_fun( var_less ) );
	}
	
	
	environ_store* environ_store::pop( environ_store* top )
	{
		ASSERT( top != NULL );
		
		environ_store *const next = top->its_next;
		
		ASSERT( next != NULL );
		
		next->update_environ();
		
		delete top;
		
		return next;
	}
	
	environ_store::environ_store( environ_store* next, char** envp )
	:
		its_next( next )
	{
		copy_vars( envp, its_vars );
		
		update_environ();
	}
	
	environ_store::~environ_store()
	{
		reset();
	}
	
	void environ_store::update_environ()
	{
		environ = &its_vars.front();
	}
	
	void environ_store::preallocate()
	{
		// We reserve an extra slot so we can later insert without allocating memory, which
		// (a) could fail and throw bad_alloc, or
		// (b) could succeed and invalidate iterators.
		
		its_vars.push_back( NULL );
		its_vars.pop_back();
		
		update_environ();
	}
	
	void environ_store::overwrite( std::vector< char* >::iterator  it,
	                               char                           *string,
	                               bool                            putting )
	{
		// true for putenv(), false for setenv(), known at compile time.
		const bool new_is_user_owned = putting;
		
		char *const var = *it;
		
		std::set< const char* >::iterator user_ownership = its_user_owned_vars.find( var );
		
		// true for putenv(), false for setenv(), known at runtime.
		const bool old_is_user_owned = user_ownership != its_user_owned_vars.end();
		
		// User-owned var strings don't get allocated or deallocated here,
		// but instead we have to mark them so we don't delete them later.
		
		if ( new_is_user_owned )
		{
			its_user_owned_vars.insert( string );  // may throw
		}
		
		*it = string;
		
		if ( old_is_user_owned )
		{
			its_user_owned_vars.erase( user_ownership );
		}
		else
		{
			delete [] var;
		}
	}
	
	void environ_store::reset()
	{
		// Here we zero out user-owned var string storage.  This is a convenience
		// that allows us to subsequently call delete_vars() safely without
		// giving it a dependency on the user ownership structure.
		
		for ( std::vector< char* >::iterator it = its_vars.begin();  it != its_vars.end();  ++it )
		{
			std::set< const char* >::iterator user_ownership = its_user_owned_vars.find( *it );
			
			if ( user_ownership != its_user_owned_vars.end() )
			{
				*it = NULL;
			}
		}
		
		its_user_owned_vars.clear();
		
		delete_vars( its_vars );
	}
	
	char* environ_store::get( const char* name )
	{
		std::vector< char* >::iterator it = find_var( its_vars, name );
		
		char *const var = *it;
		
		return var_match( var, name );
	}
	
	static char* copy_var( const char* name, std::size_t name_length, const char* value, std::size_t value_length )
	{
		const std::size_t total_length = name_length + 1 + value_length;
		
		char *const result = new char[ total_length + 1 ];
		
		std::memcpy( result, name, name_length );
		
		result[ name_length ] = '=';
		
		std::memcpy( result + name_length + 1, value, value_length + 1 );
		
		return result;
	}
	
	void environ_store::set( const char* name, const char* value, bool overwriting )
	{
		preallocate();  // make insertion safe
		
		std::vector< char* >::iterator it = find_var( its_vars, name );
		
		const char *const var = *it;
		
		// Did we find the right environment variable?
		const char *const match = var_match( var, name );
		
		// If it doesn't match, we insert (otherwise, we possibly overwrite)
		const bool inserting = !match;
		
		if ( inserting )
		{
			char *const new_var = copy_var( name, std::strlen( name ), value, std::strlen( value ) );
			
			its_vars.insert( it, new_var );  // won't throw
		}
		else if ( overwriting )
		{
			const std::size_t name_length  = match - var - 1;
			const std::size_t value_length = std::strlen( value );
			
			char *const new_var = copy_var( name, name_length, value, value_length );
			
			overwrite( it, new_var, false );
		}
	}
	
	void environ_store::put( char* string )
	{
		preallocate();  // make insertion safe
		
		std::vector< char* >::iterator it = find_var( its_vars, string );
		
		const char *const var = *it;
		
		// Did we find the right environment variable?
		const char *const match = var_match( var, string );
		
		// If it doesn't match, we insert (otherwise, we possibly overwrite)
		const bool inserting = !match;
		
		if ( inserting )
		{
			its_user_owned_vars.insert( string );  // may throw
			
			its_vars.insert( it, string );  // memory already reserved
		}
		else
		{
			overwrite( it, string, true );
		}
	}
	
	void environ_store::unset( const char* name )
	{
		std::vector< char* >::iterator it = find_var( its_vars, name );
		
		const char *const var = *it;
		
		// Did we find the right environment variable?
		const bool match = var_match( var, name );
		
		
		if ( match )
		{
			std::set< const char* >::iterator user_ownership = its_user_owned_vars.find( var );
			
			const bool user_owned = user_ownership != its_user_owned_vars.end();
			
			if ( user_owned )
			{
				its_user_owned_vars.erase( user_ownership );
			}
			
			its_vars.erase( it );
			
			delete [] var;
		}
	}
	
	void environ_store::clear()
	{
		reset();
		
		its_vars.clear();
		
		its_vars.resize( 1, NULL );
		
		environ = NULL;
	}
	
}

