// Nitrogen/TextUtils.hh
// ---------------------
//
// Maintained by Joshua Juran

// Part of the Nitrogen project.
//
// Written 2005 by Joshua Juran.
//
// This code was written entirely by the above contributor, who places it
// in the public domain.


#ifndef NITROGEN_TEXTUTILS_HH
#define NITROGEN_TEXTUTILS_HH

// Mac OS X
#ifdef __APPLE__
#include <CoreServices/CoreServices.h>
#endif

// Mac OS
#ifndef __TEXTUTILS__
#include <TextUtils.h>
#endif

// Nitrogen
#ifndef MAC_RESOURCES_TYPES_RESID_HH
#include "Mac/Resources/Types/ResID.hh"
#endif

#ifndef NITROGEN_MACMEMORY_HH
#include "Nitrogen/MacMemory.hh"
#endif
#ifndef NITROGEN_STR_HH
#include "Nitrogen/Str.hh"
#endif


namespace Nitrogen
{
	
	// Munger
	// I am *not* porting Munger().  There are C++ algorithms to do the same things type-safely.
	
#if ! __LP64__
	
	nucleus::owned< StringHandle > NewString( ConstStr255Param string );
	
	void SetString( StringHandle string, ConstStr255Param newStr );
	
	StringHandle GetString( Mac::ResID stringID );
	
	Str255 GetIndString( Mac::ResID resID, UInt16 index );
	
#endif  // #if ! __LP64__
	
	inline Str255 c2pstrcpy( const char* src )  { return Str255( src ); }
	
	nucleus::string p2cstrcpy( ConstStr255Param src );
	
	inline nucleus::string CopyPascalStringToC( ConstStr255Param src )  { return p2cstrcpy( src ); }
	inline Str255          CopyCStringToPascal( const char*      src )  { return c2pstrcpy( src ); }
	
}

#endif
