// Nitrogen/TextUtils.cc
// ---------------------
//
// Maintained by Joshua Juran

// Part of the Nitrogen project.
//
// Written 2005 by Joshua Juran.
//
// This code was written entirely by the above contributor, who places it
// in the public domain.


#include "Nitrogen/TextUtils.hh"


namespace Nitrogen
{
	
	nucleus::owned< StringHandle > NewString( ConstStr255Param string )
	{
		StringHandle result = ::NewString( string );
		
		MemError();
		
		return nucleus::owned< StringHandle >::seize( result );
	}
	
	void SetString( StringHandle string, ConstStr255Param newStr )
	{
		::SetString( string, newStr );
		
		MemError();
	}
	
	StringHandle GetString( Mac::ResID stringID )
	{
		StringHandle result = ::GetString( stringID );
		
		ResError();
		
		return result;
	}
	
	Str255 GetIndString( Mac::ResID resID, UInt16 index )
	{
		::Str255 str;
		::GetIndString( str, resID, index );
		
		ResError();
		
		return str;
	}
	
	nucleus::string p2cstrcpy( ConstStr255Param src )
	{
		return nucleus::string( (const char*) &src[1], src[0] );
	}
	
}
