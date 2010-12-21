// Nitrogen/HIShape.hh
// -------------------
//
// Maintained by Marshall Clow

// Part of the Nitrogen project.
//
// Written 2004-2006 by Marshall Clow and Joshua Juran.
//
// This code was written entirely by the above contributors, who place it
// in the public domain.


#ifndef NITROGEN_HISHAPE_HH
#define NITROGEN_HISHAPE_HH

#ifndef __MACH__
#error "These routines are only directly callable from MachO"
#endif

#ifndef __HISHAPE__
#include <Carbon/Carbon.h>
#endif

#ifndef NITROGEN_CFBASE_HH
#include "Nitrogen/CFBase.hh"
#endif

#ifndef NITROGEN_OSSTATUS_HH
#include "Nitrogen/OSStatus.hh"
#endif

#ifndef NITROGEN_QUICKDRAW_HH
#include "Nitrogen/Quickdraw.hh"
#endif


namespace Nitrogen {

	class HIShapeErrorsRegistrationDependency
	{
		public:
			HIShapeErrorsRegistrationDependency();
	};
	
	inline HIShapeErrorsRegistrationDependency::HIShapeErrorsRegistrationDependency()
	{
		// Apple hasn't documented any HIShape errors.
	}
	
  }

namespace nucleus
  {
	template <> struct disposer_class< HIShapeRef        >: disposer_class< Nitrogen::CFTypeRef >  {};
	template <> struct disposer_class< HIMutableShapeRef >: disposer_class< Nitrogen::CFTypeRef >  {};
  }

namespace Nitrogen
  {	

	template <> struct CFType_Traits< HIMutableShapeRef >: Basic_CFType_Traits< HIMutableShapeRef, ::HIShapeGetTypeID > {};
	template <> struct CFType_Traits< HIShapeRef >: Basic_CFType_Traits< HIShapeRef, ::HIShapeGetTypeID > {};

//	extern CFTypeID HIShapeGetTypeID(void)
	using ::HIShapeGetTypeID;

/*======================================================================================*/
/*  IMMUTABLE FUNCTIONS                                                                 */
/*======================================================================================*/

	class HIShapeCreateWithQDRgn_Failed {};
	nucleus::owned<HIShapeRef> HIShapeCreateWithQDRgn ( RgnHandle inRgn );

	class HIShapeCreateWithRect_Failed {};
	nucleus::owned<HIShapeRef> HIShapeCreateWithRect ( const HIRect &inRect );

	class HIShapeCreateCopy_Failed {};
	nucleus::owned<HIShapeRef> HIShapeCreateCopy ( HIShapeRef inShape );

	class HIShapeCreateIntersection_Failed {};
	nucleus::owned<HIShapeRef> HIShapeCreateIntersection ( HIShapeRef inShape1, HIShapeRef inShape2 );

	class HIShapeCreateDifference_Failed {};
	nucleus::owned<HIShapeRef> HIShapeCreateDifference ( HIShapeRef inShape1, HIShapeRef inShape2 );

	class HIShapeCreateUnion_Failed {};
	nucleus::owned<HIShapeRef> HIShapeCreateUnion ( HIShapeRef inShape1, HIShapeRef inShape2 );

//	extern Boolean HIShapeIsEmpty(HIShapeRef inShape)
	using ::HIShapeIsEmpty;
	
//	extern Boolean HIShapeIsRectangular(HIShapeRef inShape)
	using ::HIShapeIsRectangular;

	inline bool HIShapeContainsPoint ( HIShapeRef inShape, const HIPoint &inPoint ) {
		return ::HIShapeContainsPoint ( inShape, &inPoint );
		}
	
	inline HIRect HIShapeGetBounds ( HIShapeRef inShape ) {
		HIRect result;
		(void) ::HIShapeGetBounds ( inShape, &result );
		return result;
		}
	
	inline nucleus::owned<RgnHandle> HIShapeGetAsQDRgn ( HIShapeRef inShape );
	
	inline void HIShapeReplacePathInCGContext ( HIShapeRef inShape, CGContextRef inContext ) {
     	(void) HIShapeErrorsRegistrationDependency();
		ThrowOSStatus ( ::HIShapeReplacePathInCGContext ( inShape, inContext ));
		}

	inline void HIShapeSetQDClip ( HIShapeRef inShape, CGrafPtr inPort ) {
     	(void) HIShapeErrorsRegistrationDependency();
		ThrowOSStatus ( ::HIShapeSetQDClip ( inShape, inPort ));
		}

/*======================================================================================*/
/*  MUTABLE FUNCTIONS                                                                   */
/*======================================================================================*/

	class HIShapeCreateMutable_Failed {};
	nucleus::owned<HIMutableShapeRef> HIShapeCreateMutable ( void );

	class HIShapeCreateMutableCopy_Failed {};
	nucleus::owned<HIMutableShapeRef> HIShapeCreateMutableCopy ( HIShapeRef inOrig );

	inline void HIShapeSetEmpty ( HIMutableShapeRef inShape ) {
     	(void) HIShapeErrorsRegistrationDependency();
		ThrowOSStatus ( ::HIShapeSetEmpty ( inShape ));
		}
	
	inline void HIShapeIntersect ( HIShapeRef inShape1, HIShapeRef inShape2, HIMutableShapeRef outResult ) {
     	(void) HIShapeErrorsRegistrationDependency();
		ThrowOSStatus ( ::HIShapeIntersect ( inShape1, inShape2, outResult ));
		}

	inline void HIShapeDifference ( HIShapeRef inShape1, HIShapeRef inShape2, HIMutableShapeRef outResult ) {
     	(void) HIShapeErrorsRegistrationDependency();
		ThrowOSStatus ( ::HIShapeDifference ( inShape1, inShape2, outResult ));
		}

	inline void HIShapeUnion ( HIShapeRef inShape1, HIShapeRef inShape2, HIMutableShapeRef outResult ) {
     	(void) HIShapeErrorsRegistrationDependency();
		ThrowOSStatus ( ::HIShapeUnion ( inShape1, inShape2, outResult ));
		}

	inline void HIShapeOffset ( HIMutableShapeRef inShape, float inDX, float inDY ) {
     	(void) HIShapeErrorsRegistrationDependency();
		ThrowOSStatus ( ::HIShapeOffset ( inShape, inDX, inDY ));
		}

	}


#endif /* NITROGEN_HISHAPE_HH */

