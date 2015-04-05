// Nitrogen/Quickdraw.cc
// ---------------------
//
// Maintained by Joshua Juran

// Part of the Nitrogen project.
//
// Written 2002-2006 by Lisa Lippincott, Marshall Clow, and Joshua Juran.
//
// This code was written entirely by the above contributors, who place it
// in the public domain.


#include "Nitrogen/Quickdraw.hh"

// Nitrogen
#include "Nitrogen/MacErrors.hh"
#include "Nitrogen/OSStatus.hh"
#include "Nitrogen/Resources.hh"

#if TARGET_RT_MAC_MACHO || TARGET_API_MAC_OSX
#include "Nitrogen/CGDirectDisplay.hh"
#endif


namespace Nitrogen
{
	
	// does nothing, but guarantees construction of theRegistration
	NUCLEUS_DEFINE_ERRORS_DEPENDENCY( QuickDraw )
	
	
	static void RegisterQuickDrawErrors();
	
	
#if NUCLEUS_RICH_ERRORCODES
#pragma force_active on
	
	class QuickDrawErrorsRegistration
	{
		public:
			QuickDrawErrorsRegistration()  { RegisterQuickDrawErrors(); }
	};
	
	static QuickDrawErrorsRegistration theRegistration;
	
#pragma force_active reset
#endif
	
	
	GrafPtr GetPort()
	{
		::GrafPtr port;
		::GetPort( &port );
		
		return port;
	}
	
	CGrafPtr QDSwapPort( CGrafPtr newPort )
	{
		CGrafPtr oldPort;
		(void) QDSwapPort( newPort, oldPort );
		
		return oldPort;
	}
	
	nucleus::owned< RgnHandle > GetClip()
	{
		nucleus::owned< RgnHandle > result = NewRgn();
		::GetClip( result.get() );
		
		return result;
	}
	
	void MacSetCursor( Mac::ResID id )
	{
		MacSetCursor( MacGetCursor( id ) );
	}
	
	PenState GetPenState()
	{
		PenState penState;
		::GetPenState( &penState );
		
		return penState;
	}
	
	Rect MacOffsetRect( const Rect& r, short dh, short dv )
	{
		return MacSetRect( r.left   + dh,
		                   r.top    + dv,
		                   r.right  + dh,
		                   r.bottom + dv );
	}
	
	Rect MacInsetRect( const Rect& r, short dh, short dv )
	{
		return MacSetRect( r.left   + dh,
		                   r.top    + dv,
		                   r.right  - dh,
		                   r.bottom - dv );
	}
	
	SectRect_Result SectRect( const Rect& a, const Rect& b )
	{
		SectRect_Result result;
		result.intersected = ::SectRect( &a, &b, &result.intersection );
		
		return result;
	}
	
	Rect MacUnionRect( const Rect& a, const Rect& b )
	{
		Rect result;
		::MacUnionRect( &a, &b, &result );
		
		return result;
	}
	
	nucleus::owned< RgnHandle > NewRgn(void)
	{
		RgnHandle result = ::NewRgn();
		
		if ( result == NULL )
		{
			throw MemFullErr();
		}
		
		return nucleus::owned< RgnHandle >::seize( result );
	}
	
	nucleus::owned< RgnHandle > CloseRgn()
	{
		nucleus::owned< RgnHandle > result = NewRgn();
		
		::CloseRgn( result.get() );
		
		return result;
	}
	
	nucleus::owned< RgnHandle > MacCopyRgn( RgnHandle srcRgn )
	{
		nucleus::owned< RgnHandle > result = NewRgn();
		::MacCopyRgn( srcRgn, result );
		
		return result;
	}
	
	RgnHandle RectRgn( RgnHandle region, const Rect& rect )
	{
		::RectRgn( region, &rect );
		
		return region;
	}
	
	nucleus::owned< RgnHandle > RectRgn( const Rect& rect )
	{
		nucleus::owned< RgnHandle > result = NewRgn();
		::RectRgn( result, &rect );
		
		return result;
	}
	
	nucleus::owned< RgnHandle > SectRgn( RgnHandle a, RgnHandle b )
	{
		nucleus::owned< RgnHandle > result = NewRgn();
		::SectRgn( a, b, result );
		
		return result;
	}
	
	nucleus::owned< RgnHandle > MacUnionRgn( RgnHandle a, RgnHandle b )
	{
		nucleus::owned< RgnHandle > result = NewRgn();
		::MacUnionRgn( a, b, result );
		
		return result;
	}
	
	nucleus::owned< RgnHandle > DiffRgn( RgnHandle a, RgnHandle b )
	{
		nucleus::owned< RgnHandle > result = NewRgn();
		::DiffRgn( a, b, result );
		
		return result;
	}
	
	nucleus::owned< RgnHandle > MacXorRgn( RgnHandle a, RgnHandle b )
	{
		nucleus::owned< RgnHandle > result = NewRgn();
		::MacXorRgn( a, b, result );
		
		return result;
	}
	
	void ScrollRect( const Rect& r, short dh, short dv, RgnHandle updateRgn )
	{
		::ScrollRect( &r, dh, dv, updateRgn );
	}
	
	nucleus::owned< RgnHandle > ScrollRect( const Rect& r, short dh, short dv )
	{
		nucleus::owned< RgnHandle > region = NewRgn();
		ScrollRect( r, dh, dv, region );
		
		return region;
	}
	
	void CopyBits( const BitMap*  srcBits,
	               const BitMap*  dstBits,
	               const Rect&    srcRect,
	               const Rect&    dstRect,
	               TransferMode   mode,
	               RgnHandle      maskRgn )
	{
		::CopyBits( srcBits, dstBits, &srcRect, &dstRect, mode, maskRgn );
	}
	
	Point LocalToGlobal( Point point )
	{
		::LocalToGlobal( &point );
		
		return point;
	}
	
	Rect LocalToGlobal( const Rect& rect )
	{
		return MacSetRect( LocalToGlobal( SetPt( rect.left,  rect.top    ) ),
		                   LocalToGlobal( SetPt( rect.right, rect.bottom ) ) );
	}
	
	Point GlobalToLocal( Point point )
	{
		::GlobalToLocal( &point );
		
		return point;
	}
	
	Rect GlobalToLocal( const Rect& rect )
	{
		return MacSetRect( GlobalToLocal( SetPt( rect.left,  rect.top    ) ),
		                   GlobalToLocal( SetPt( rect.right, rect.bottom ) ) );
	}
	
	Point AddPt( Point a, Point b )
	{
		return SetPt( a.h + b.h,
		              a.v + b.v );
	}
	
	Point SubPt( Point a, Point b )
	{
		return SetPt( a.h - b.h,
		              a.v - b.v );
	}
	
	RGBColor GetCPixel( short h, short v )
	{
		RGBColor result;
		::GetCPixel( h, v, &result );
		
		return result;
	}
	
	RGBColor GetForeColor()
	{
		RGBColor result;
		::GetForeColor( &result );
		
		return result;
	}
	
	RGBColor GetBackColor()
	{
		RGBColor result;
		::GetBackColor( &result );
		
		return result;
	}
	
	void QDError()
	{
		Mac::ThrowOSStatus( ::QDError() );
	}
	
	CursHandle MacGetCursor( Mac::ResID id )
	{
		return CheckResource( ::MacGetCursor( id ) );
	}
	
	const BitMap* GetPortBitMapForCopyBits( CGrafPtr port )
	{
		return ::GetPortBitMapForCopyBits( port );
	}
	
	Rect GetPortBounds( CGrafPtr port )
	{
		Rect bounds;
		return *( ::GetPortBounds( port, &bounds ) );
	}
	
	RGBColor GetPortForeColor( CGrafPtr port )
	{
		RGBColor result;
		::GetPortForeColor( port, &result );
		
		return result;
	}
	
	RGBColor GetPortBackColor( CGrafPtr port )
	{
		RGBColor result;
		::GetPortBackColor( port, &result );
		
		return result;
	}
	
	RgnHandle GetPortVisibleRegion( CGrafPtr port, RgnHandle region )
	{
		return ::GetPortVisibleRegion( port, region );
	}
	
	nucleus::owned< RgnHandle > GetPortVisibleRegion( CGrafPtr port )
	{
		nucleus::owned< RgnHandle > region = NewRgn();
		(void)Nitrogen::GetPortVisibleRegion( port, region );
		
		return region;
	}
	
	RgnHandle GetPortClipRegion( CGrafPtr port, RgnHandle region )
	{
		return ::GetPortClipRegion( port, region );
	}
	
	nucleus::owned< RgnHandle > GetPortClipRegion( CGrafPtr port )
	{
		nucleus::owned< RgnHandle > region = NewRgn();
		Nitrogen::GetPortClipRegion( port, region );
		
		return region;
	}
	
	Point GetPortPenSize( CGrafPtr port )
	{
		Point result;
		(void)::GetPortPenSize( port, &result );
		
		return result;
	}
	
	bool IsPortColor( CGrafPtr port )
	{
		return ::IsPortColor( port );
	}
	
	void SetPortClipRegion( CGrafPtr port, RgnHandle clipRgn )
	{
		::SetPortClipRegion( port, clipRgn );
	}
	
	void SetPortPenSize( CGrafPtr port, Point penSize )
	{
		::SetPortPenSize( port, penSize );
	}
	
#if ACCESSOR_CALLS_ARE_FUNCTIONS
	
	namespace Detail
	{
		
		template < class Type >
		Type& QDGlobalsVar
		(
		#if !TARGET_RT_MAC_MACHO
			pascal
		#endif
			Type* (getter)(Type*)
		)
		{
			static Type var;
			return *getter( &var );
		}
		
	}
	
	
	// Thread-safety note:
	//
	// GetQDGlobalsScreenBits() and GetQDGlobalsArrow() should be okay,
	// since two simultaneous accesses would be copying the same data.
	// The Pattern-returning functions are not thread-safe.
	
	const BitMap&  GetQDGlobalsScreenBits()  { return Detail::QDGlobalsVar( ::GetQDGlobalsScreenBits ); }
	const Cursor&  GetQDGlobalsArrow()       { return Detail::QDGlobalsVar( ::GetQDGlobalsArrow      ); }
	const Pattern& GetQDGlobalsDarkGray()    { return Detail::QDGlobalsVar( ::GetQDGlobalsDarkGray   ); }
	const Pattern& GetQDGlobalsLightGray()   { return Detail::QDGlobalsVar( ::GetQDGlobalsLightGray  ); }
	const Pattern& GetQDGlobalsGray()        { return Detail::QDGlobalsVar( ::GetQDGlobalsGray       ); }
	const Pattern& GetQDGlobalsBlack()       { return Detail::QDGlobalsVar( ::GetQDGlobalsBlack      ); }
	const Pattern& GetQDGlobalsWhite()       { return Detail::QDGlobalsVar( ::GetQDGlobalsWhite      ); }
	
#endif
	
	nucleus::owned< CGrafPtr > CreateNewPort()
	{
		return nucleus::owned< CGrafPtr >::seize( ::CreateNewPort(),
		                                          &::DisposePort );
	}
	
#if TARGET_API_MAC_OSX
	
	// Declared in Nitrogen/CGDirectDisplay.hh, since it depends on CGDirectDisplayID
	
	nucleus::owned< CGrafPtr > CreateNewPortForCGDisplayID( CGDirectDisplayID display )
	{
		UInt32 id = (UInt32) display;
		
		return nucleus::owned< CGrafPtr >::seize( ::CreateNewPortForCGDisplayID( id ),
		                                          &::DisposePort );
	}
	
#endif
	
	void RegisterQuickDrawErrors()
	{
		NUCLEUS_REQUIRE_ERRORS( MemoryManager );
		
		// CopyBits couldn't allocate required temporary memory
		RegisterOSStatus< -143                    >();
		
		// Ran out of stack space while drawing polygon
		RegisterOSStatus< -144                    >();
		
		RegisterOSStatus< noMemForPictPlaybackErr >();
		RegisterOSStatus< rgnOverflowErr          >();
		RegisterOSStatus< pixMapTooDeepErr        >();
		RegisterOSStatus< insufficientStackErr    >();
		RegisterOSStatus< cMatchErr               >();
		RegisterOSStatus< cTempMemErr             >();
		RegisterOSStatus< cNoMemErr               >();
		RegisterOSStatus< cRangeErr               >();
		RegisterOSStatus< cProtectErr             >();
		RegisterOSStatus< cDevErr                 >();
		RegisterOSStatus< cResErr                 >();
		RegisterOSStatus< cDepthErr               >();
		RegisterOSStatus< rgnTooBigErr            >();
		RegisterOSStatus< updPixMemErr            >();
		RegisterOSStatus< pictInfoVersionErr      >();
		RegisterOSStatus< pictInfoIDErr           >();
		RegisterOSStatus< pictInfoVerbErr         >();
		RegisterOSStatus< cantLoadPickMethodErr   >();
		RegisterOSStatus< colorsRequestedErr      >();
		RegisterOSStatus< pictureDataErr          >();
	}
  
}
