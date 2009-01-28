/*	================
 *	Scroller_beta.hh
 *	================
 */

#ifndef PEDESTAL_SCROLLERBETA_HH
#define PEDESTAL_SCROLLERBETA_HH

// Pedestal
#include "Pedestal/CustomTEClickLoop.hh"
#include "Pedestal/Superview.hh"


namespace Pedestal
{
	
	class Scrollbar;
	
	
	class ScrollerAPI
	{
		public:
			virtual short ViewWidth () const = 0;
			virtual short ViewHeight() const = 0;
			
			virtual int ClientWidth () const = 0;
			virtual int ClientHeight() const = 0;
			
			virtual int GetHOffset() const = 0;
			virtual int GetVOffset() const = 0;
			
			virtual void SetHOffset( int h ) = 0;
			virtual void SetVOffset( int v ) = 0;
	};
	
	class Scroller : public Superview, public ScrollerAPI
	{
		public:
			bool KeyDown( const EventRecord& event );
			
			virtual void Scroll( int dh, int dv ) = 0;
	};
	
	class ScrollFrame : public Superview, public TEClickLoop_User
	{
		public:
			virtual void ApertureHook( Rect& aperture )  {}
			
			virtual void UpdateScrollbars()  {}
			
			virtual const Rect& Bounds() const = 0;
			
			virtual Scrollbar& GetHorizontal() = 0;
			virtual Scrollbar& GetVertical  () = 0;
			
			void ClickInLoop();
			
			Rect GetAperture();
			
			void Activate( bool activating );
			
			bool HitTest( const EventRecord& event );
			
			void MouseDown( const EventRecord& event );
			
			bool KeyDown( const EventRecord& event );
			
			bool UserCommand( MenuItemCode code );
			
			void Draw( const Rect& bounds );
	};
	
}

#endif

