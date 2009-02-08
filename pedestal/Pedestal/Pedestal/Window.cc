/*	============
 *	PedWindow.cc
 *	============
 */

#include "Pedestal/Window.hh"

// Nucleus
#include "Nucleus/Saved.h"

// ClassicToolbox
#include "ClassicToolbox/MacWindows.h"

// Pedestal
#include "Pedestal/View.hh"


namespace Pedestal
{
	
	namespace N = Nitrogen;
	namespace NN = Nucleus;
	
	
	void ResizeWindow( N::WindowRef window, Point newSize )
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
	
	void SetWindowSize( Nitrogen::WindowRef window, Point size )
	{
		NN::Saved< N::Port_Value > savePort;
		
		N::SetPortWindowPort( window );
		
		ResizeWindow( window, size );
	}
	
	
	Point GetWindowSize( N::WindowRef window )
	{
		Rect bounds = N::GetPortBounds( N::GetWindowPort( window ) );
		
		Point result;
		
		result.h = bounds.right - bounds.left;
		result.v = bounds.bottom - bounds.top;
		
		return result;
	}
	
	Point GetWindowPosition( N::WindowRef window )
	{
		NN::Saved< N::Port_Value > savedPort;
		
		N::SetPortWindowPort( window );
		
		Point upperLeft = { 0, 0 };
		
		::LocalToGlobal( &upperLeft );
		
		return upperLeft;
	}
	
	
	NN::Owned< N::WindowRef > CreateWindow( const Rect&         bounds,
	                                        ConstStr255Param    title,
	                                        bool                visible,
	                                        N::WindowDefProcID  procID,
	                                        N::WindowRef        behind,
	                                        bool                goAwayFlag,
	                                        N::RefCon           refCon )
	{
		NN::Owned< N::WindowRef > window = N::NewCWindow( bounds,
		                                                  title,
		                                                  visible,
		                                                  procID,
		                                                  behind,
		                                                  goAwayFlag,
		                                                  refCon );
		
		//N::SetWindowKind( window, kPedestalWindowKind );
		N::SetPortWindowPort( window );
		
		return window;
	}
	
	static Rect GrowBoxBounds( N::WindowRef window )
	{
		Rect bounds = N::GetPortBounds( N::GetWindowPort( window ) );
		
		bounds.left = bounds.right - 15;
		bounds.top = bounds.bottom - 15;
		
		return bounds;
	}
	
	static void DrawWindow( N::WindowRef window )
	{
		NN::Saved< N::Clip_Value > savedClip;
		
		N::ClipRect( GrowBoxBounds( window ) );
		
		N::DrawGrowIcon( window );
	}
	
	void InvalidateWindowGrowBox( N::WindowRef window )
	{
		N::InvalRect( GrowBoxBounds( window ) );
	}
	
	
	void Window::Resize( Nitrogen::WindowRef window, short h, short v )
	{
		InvalidateWindowGrowBox( window );  // assume grow box present on resize
		
		if ( const WindowResizeHandler* handler = itsResizeHandler.get() )
		{
			(*handler)( window, h, v );
		}
		else
		{
			Nitrogen::SizeWindow( window, h, v, true );
		}
	}
	
	
	Window::Window( const NewWindowContext& context )
	:
		itsWindowRef( CreateWindow( context, this ) ),
		itsDefProcID( context.procID )
	{
	}
	
	Window::~Window()
	{
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
		GetView()->Draw( Nitrogen::GetPortBounds( Nitrogen::GetWindowPort( Get() ) ) );
		
		if ( !TARGET_API_MAC_CARBON && HasGrowIcon() )
		{
			DrawWindow( Get() );
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

