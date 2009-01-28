/*	================
 *	Scroller_beta.cc
 *	================
 */

#include "Pedestal/Scroller_beta.hh"

// Nucleus
#include "Nucleus/Saved.h"


namespace Pedestal
{
	
	namespace N = Nitrogen;
	namespace NN = Nucleus;
	
	
	static int NewVOffset( const Scroller& scroller, char c )
	{
		if ( c == kHomeCharCode )
		{
			return 0;
		}
		
		const int max_voffset = std::max( scroller.ClientHeight() - scroller.ViewHeight(), 0 );
		
		if ( c == kEndCharCode )
		{
			return max_voffset;
		}
		
		const int voffset = scroller.GetVOffset();
		
		const short page_distance = scroller.ViewHeight() - 1;
		
		switch ( c )
		{
			case kPageUpCharCode:
				return std::max( voffset - page_distance, 0 );
			
			case kPageDownCharCode:
				return std::min( voffset + page_distance, max_voffset );
			
			default:
				break;
		}
		
		return -1;
	}
	
	bool Scroller::KeyDown( const EventRecord& event )
	{
		const char keyCode = (event.message & keyCodeMask) >> 8;
		
		// Only consider programmers' keys, not control characters
		if ( keyCode >= 0x70 )
		{
			const char keyChar = event.message & charCodeMask;
			
			const int voffset = NewVOffset( *this, keyChar );
			
			if ( voffset >= 0 )
			{
				SetVOffset( voffset );
				
				return true;
			}
		}
		
		return Superview::KeyDown( event );
	}
	
	
	void ScrollFrame::ClickInLoop()
	{
		NN::Saved< N::Clip_Value > savedClip;
		
		N::ClipRect( N::GetPortBounds( N::GetQDGlobalsThePort() ) );
		
		UpdateScrollbars();
	}
	
	bool ScrollFrame::HitTest( const EventRecord& event )
	{
		return N::PtInRect( N::GlobalToLocal( event.where ), GetAperture() );
	}
	
	void ScrollFrame::MouseDown( const EventRecord& event )
	{
		if ( HitTest( event ) )
		{
			TEClickLoop_Scope scope( this );
			
			Subview().MouseDown( event );
			
			UpdateScrollbars();
		}
	}
	
	bool ScrollFrame::KeyDown( const EventRecord& event )
	{
		const bool result = Subview().KeyDown( event );
		
		if ( result )
		{
			UpdateScrollbars();
		}
		
		return result;
	}
	
	bool ScrollFrame::UserCommand( MenuItemCode code )
	{
		const bool result = Subview().UserCommand( code );
		
		if ( result )
		{
			UpdateScrollbars();
		}
		
		return result;
	}
	
	void ScrollFrame::Draw( const Rect& bounds )
	{
		Rect aperture = bounds;
		
		ApertureHook( aperture );
		
		Subview().Draw( aperture );
	}
	
}

