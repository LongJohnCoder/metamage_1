/*	============
 *	PedWindow.cc
 *	============
 */

#include "Pedestal/Window.hh"

// MacGlue
#include "MacGlue/MacGlue.hh"

// nucleus
#include "nucleus/saved.hh"

// Nitrogen
#include "Nitrogen/Quickdraw.hh"

// ClassicToolbox
#include "ClassicToolbox/MacWindows.hh"

// MacFeatures
#include "MacFeatures/ColorQuickdraw.hh"

// Pedestal
#include "Pedestal/View.hh"
#include "Pedestal/WindowMenu.hh"


namespace MacGlue
{
	
	DECLARE_MAC_GLUE( NewWindow  );
	DECLARE_MAC_GLUE( NewCWindow );
	
}

namespace Pedestal
{
	
	namespace n = nucleus;
	namespace N = Nitrogen;
	
	
	typedef pascal WindowRef (*NewWindow_ProcPtr)( void*             storage,
	                                               const Rect*       bounds,
	                                               ConstStr255Param  title,
	                                               Boolean           visible,
	                                               short             procID,
	                                               WindowRef         behind,
	                                               Boolean           goAwayFlag,
	                                               long              refCon );
	
	using MacFeatures::Has_ColorQuickdraw;
	
	static const NewWindow_ProcPtr gNewWindow = Has_ColorQuickdraw() ? &MacGlue::NewCWindow
	                                                                 : &MacGlue::NewWindow;
	
	
	void ResizeWindow( WindowRef window, Point newSize )
	{
		if ( Window* base = N::GetWRefCon( window ) )
		{
			// Resize the window
			base->Resize( window, newSize.h, newSize.v );
			
			// Don't rely on the requested size because it might have been tweaked
			Rect bounds = N::GetPortBounds( N::GetWindowPort( window ) );
			
			// Shotgun approach -- invalidate the whole window.
			// Clients can validate regions if they want.
			N::InvalRect( bounds );
			
			// Inform the window's contents that it has been resized
			base->Resized( bounds.right, bounds.bottom );
		}
	}
	
	void SetWindowSize( WindowRef window, Point size )
	{
		n::saved< N::Port > savePort;
		
		N::SetPortWindowPort( window );
		
		ResizeWindow( window, size );
	}
	
	
	Point GetWindowSize( WindowRef window )
	{
		Rect bounds = N::GetPortBounds( N::GetWindowPort( window ) );
		
		Point result;
		
		result.h = bounds.right - bounds.left;
		result.v = bounds.bottom - bounds.top;
		
		return result;
	}
	
	Point GetWindowPosition( WindowRef window )
	{
		n::saved< N::Port > savedPort;
		
		N::SetPortWindowPort( window );
		
		Point upperLeft = { 0, 0 };
		
		::LocalToGlobal( &upperLeft );
		
		return upperLeft;
	}
	
	
	n::owned< WindowRef > CreateWindow( const Rect&         bounds,
	                                    ConstStr255Param    title,
	                                    bool                visible,
	                                    N::WindowDefProcID  procID,
	                                    WindowRef           behind,
	                                    bool                goAwayFlag,
	                                    const void*         refCon )
	{
		
		WindowRef window = gNewWindow( NULL,
		                               &bounds,
		                               title,
		                               visible,
		                               procID,
		                               behind,
		                               goAwayFlag,
		                               (long) refCon );  // reinterpret_cast
		
		//N::SetWindowKind( window, kPedestalWindowKind );
		N::SetPortWindowPort( window );
		
		return n::owned< WindowRef >::seize( window, N::Window_Disposer() );
	}
	
	static Rect GrowBoxBounds( WindowRef window )
	{
		Rect bounds = N::GetPortBounds( N::GetWindowPort( window ) );
		
		bounds.left = bounds.right - 15;
		bounds.top = bounds.bottom - 15;
		
		return bounds;
	}
	
	void InvalidateWindowGrowBox( WindowRef window )
	{
		N::InvalRect( GrowBoxBounds( window ) );
	}
	
	
	Window::Window( const NewWindowContext& context )
	:
		itsWindowRef( CreateWindow( context, this ) ),
		itsDefProcID( context.procID )
	{
		window_created( itsWindowRef.get() );
	}
	
	Window::~Window()
	{
		window_removed( itsWindowRef.get() );
	}
	
	void Window::Resize( WindowRef window, short h, short v )
	{
		InvalidateWindowGrowBox( window );  // assume grow box present on resize
		
		N::SizeWindow( window, h, v, true );
		
		if ( const WindowResizeHandler* handler = itsResizeHandler.get() )
		{
			(*handler)( window, h, v );
		}
	}
	
	
	void Window::Activate( bool activating )
	{
		GetView()->Activate( activating );
		
		InvalidateGrowBox();
	}
	
	void Window::Resized( short width, short height )
	{
		Rect bounds = { 0, 0, height, width };
		
		GetView()->SetBounds( bounds );
		
		InvalidateGrowBox();
	}
	
	void Window::MouseDown( const EventRecord& event )
	{
		// FIXME:  The window may want clicks even if it's not in front.
		if ( Get() != Nitrogen::FrontWindow() )
		{
			Nitrogen::SelectWindow( Get() );
		}
		else
		{
			GetView()->MouseDown( event );
		}
	}
	
	void Window::Update()
	{
		GetView()->Draw( Nitrogen::GetPortBounds( Nitrogen::GetWindowPort( Get() ) ), true );
		
		if ( !TARGET_API_MAC_CARBON && HasGrowIcon() )
		{
			n::saved< N::Clip > savedClip;
			
			N::ClipRect( GrowBoxBounds( Get() ) );
			
			N::DrawGrowIcon( Get() );
		}
	}
	
	
	#if 0
	
	static Rect CalcWindowStructureDiff()
	{
		Rect r = { -200, -200, -100, -100 };
		unsigned char* title = "\pTest";
		bool vis = true;
		int procID = 0;
		WindowRef front = kFirstWindowOfClass;
		bool goAway = true;
		int refCon = 0;
		WindowRef windowPtr = ::NewWindow(NULL, &r, title, vis, procID, front, goAway, refCon);
		
		VRegion region;
		::GetWindowStructureRgn(windowPtr, region);
		::DisposeWindow(windowPtr);
		RgnHandle rgnH = region;
		Rect bounds = (**rgnH).rgnBBox;
		
		return SetRect(
			bounds.left   - r.left, 
			bounds.top    - r.top, 
			bounds.right  - r.right, 
			bounds.bottom - r.bottom
		);
	}
	
	static Rect WindowStructureDiff()
	{
		static Rect diff = CalcWindowStructureDiff();
		return diff;
	}
	
	#endif
	
}
