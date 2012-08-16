// Nitrogen/CFBase.hh
// ------------------

// Part of the Nitrogen project.
//
// Written 2002-2009 by Lisa Lippincott, Marshall Clow, and Joshua Juran.
//
// This code was written entirely by the above contributors, who place it
// in the public domain.


#ifndef NITROGEN_CFBASE_HH
#define NITROGEN_CFBASE_HH

#ifndef __COREFOUNDATION_CFBASE__
#ifndef __CFBASE__
#include <CFBase.h>
#endif
#endif
#ifndef __COREFOUNDATION_CFSTRING__
#ifndef __CFSTRING__
#include <CFString.h>
#endif
#endif

// nucleus
#ifndef NUCLEUS_ENUMERATIONTRAITS_HH
#include "nucleus/enumeration_traits.hh"
#endif
#ifndef NUCLEUS_FLAGOPS_HH
#include "nucleus/flag_ops.hh"
#endif
#ifndef NUCLEUS_OWNED_HH
#include "nucleus/owned.hh"
#endif

// Nitrogen
#ifndef CARBON_CF_TYPES_CFTYPEREF_HH
#include "Carbon/CF/Types/CFTypeRef.hh"
#endif


namespace Nitrogen
  {
	
	using Carbon::CFTypeID;
	
// These declarations should be scattered to the appropriate files:
//   template <> struct CFType_Traits< ::CFBagRef            >: Basic_CFType_Traits< ::CFBagRef,            ::CFBagGetTypeID            > {};
//   template <> struct CFType_Traits< ::CFSetRef            >: Basic_CFType_Traits< ::CFSetRef,            ::CFSetGetTypeID            > {};
//   template <> struct CFType_Traits< ::CFTreeRef           >: Basic_CFType_Traits< ::CFTreeRef,           ::CFTreeGetTypeID           > {};
//   template <> struct CFType_Traits< ::CFCharacterSetRef   >: Basic_CFType_Traits< ::CFCharacterSetRef,   ::CFCharacterSetGetTypeID   > {};
//   template <> struct CFType_Traits< ::CFTimeZoneRef       >: Basic_CFType_Traits< ::CFTimeZoneRef,       ::CFTimeZoneGetTypeID       > {};
//   template <> struct CFType_Traits< ::CFPlugInInstanceRef >: Basic_CFType_Traits< ::CFPlugInInstanceRef, ::CFPlugInInstanceGetTypeID > {};
//   template <> struct CFType_Traits< ::CFURLRef            >: Basic_CFType_Traits< ::CFURLRef,            ::CFURLGetTypeID            > {};
//   template <> struct CFType_Traits< ::CFXMLNodeRef        >: Basic_CFType_Traits< ::CFXMLNodeRef,        ::CFXMLNodeGetTypeID        > {};
//   template <> struct CFType_Traits< ::CFXMLTreeRef        >: Basic_CFType_Traits< ::CFXMLTreeRef,        ::CFXMLTreeGetTypeID        > {};
//   template <> struct CFType_Traits< ::CFXMLParserRef      >: Basic_CFType_Traits< ::CFXMLParserRef,      ::CFXMLParserGetTypeID      > {};

	using Carbon::CFTypeRef;

}

namespace Nitrogen {
   template < class T > bool operator==( const CFTypeRef& a, T const *const& b )   { return a == CFTypeRef( b ); }
   template < class T > bool operator!=( const CFTypeRef& a, T const *const& b )   { return a != CFTypeRef( b ); }
   
   template < class T > bool operator==( T const *const& a, const CFTypeRef& b )   { return CFTypeRef( a ) == b; }
   template < class T > bool operator!=( T const *const& a, const CFTypeRef& b )   { return CFTypeRef( a ) != b; }

	enum CFOptionFlags
	{
		kCFOptionFlags_Max = nucleus::enumeration_traits< ::CFOptionFlags >::max
	};
	
   NUCLEUS_DEFINE_FLAG_OPS( CFOptionFlags )
   
	enum CFHashCode
	{
		kCFHashCode_Max = nucleus::enumeration_traits< ::CFHashCode >::max
	};
	
}

namespace Nitrogen {

   inline CFTypeID CFGetTypeID( CFTypeRef cf )
     {
      return CFTypeID( ::CFGetTypeID( cf ) );
     }

   inline nucleus::owned<CFStringRef> CFCopyTypeIDDescription( CFTypeID cf )
     {
      return nucleus::owned<CFStringRef>::seize( ::CFCopyTypeIDDescription( cf ) );
     }
   
   template < class CF >
   nucleus::owned< CF > CFRetain( CF toRetain )
     {
      ::CFRetain( CFTypeRef( toRetain ) );
      return nucleus::owned<CF>::seize( toRetain );
     }

   inline void CFRelease( nucleus::owned<CFTypeRef> /*toRelease*/ )
     {
     }
   
   inline CFIndex CFGetRetainCount( CFTypeRef cf )
     {
      return ::CFGetRetainCount( cf );
     }

   inline bool CFEqual( CFTypeRef cf1, CFTypeRef cf2 )
     {
      return ::CFEqual( cf1, cf2 );
     }

   inline CFHashCode CFHash( CFTypeRef cf )
     {
      return CFHashCode( ::CFHash( cf ) );
     }

   inline nucleus::owned<CFStringRef> CFCopyDescription( CFTypeRef cf )
     {
      return nucleus::owned<CFStringRef>::seize( ::CFCopyDescription( cf ) );
     }

   inline CFAllocatorRef CFGetAllocator( CFTypeRef cf )
     {
      return ::CFGetAllocator( cf );
     }


   class CFCast_Failed   {};
   
   template < class Desired > Desired CFCast( CFTypeRef p )
     {
      if ( p == 0 )
         return 0;
      
      if ( CFGetTypeID( p ) != CFType_Traits<Desired>::ID() )
         throw CFCast_Failed();
      
      return static_cast< Desired >( p.Get() );
     }

   template < class Desired > nucleus::owned<Desired> CFCast( nucleus::owned<CFTypeRef> p )
     {
      Desired result = CFCast<Desired>( p.get() );
      p.release();
      return nucleus::owned<Desired>::seize( result );
     }
  }

#endif
