// ===========
// VarArray.cc
// ===========

#include "ShellShock/VarArray.hh"


namespace ShellShock
{
	
	// For input of "name" or "name=value", return a pointer to the byte following name.
	// For NULL, return NULL (identity).
	
	const char* EndOfVarName( const char* var )
	{
		if ( var == NULL )
		{
			return NULL;
		}
		
		const char* end = std::strchr( var, '=' );
		
		if ( end == NULL )
		{
			end = std::strchr( var, '\0' );
		}
		
		return end;
	}
	
	// True if the sequence (var, end) == (name, name + strlen(name)).
	// If var is NULL, it returns false.
	// end and name may be NULL if var is NULL.
	
	bool VarMatchesName( const char* var, const char* end, const char* name )
	{
		return    var != NULL
		       && end - var == std::strlen( name )
		       && std::equal( var, end, name );
	}
	
	SVector::const_iterator VarArray::Find( const char* name ) const
	{
		return std::lower_bound( array.begin(),
		                         array.end() - 1,
		                         name,
		                         std::ptr_fun( CompareStrings ) );
	}
	
	SVector::iterator VarArray::Find( const char* name )
	{
		return std::lower_bound( array.begin(),
		                         array.end() - 1,
		                         name,
		                         std::ptr_fun( CompareStrings ) );
	}
	
	void VarArray::SetVar( const char* name, const char* value, bool overwrite )
	{
		SVector::iterator it = Find( name );
		
		const char* var = *it;
		
		// Did we find the right environment variable?
		bool match = VarMatchesName( var, EndOfVarName( var ), name );
		
		bool inserting = !match;
		
		if ( inserting )
		{
			Insert( it, MakeVar( name, value ) );
		}
		else if ( overwrite )
		{
			Overwrite( it, MakeVar( name, value ) );
		}
		
		Update();
	}
	
}

