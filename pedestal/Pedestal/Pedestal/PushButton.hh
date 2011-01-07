/*	=============
 *	PushButton.hh
 *	=============
 */

#ifndef PEDESTAL_PUSHBUTTON_HH
#define PEDESTAL_PUSHBUTTON_HH

// Mac OS
#ifndef __CONTROLS__
#include <Controls.h>
#endif

// Nitrogen
#ifndef NITROGEN_STR_HH
#include "Nitrogen/Str.hh"
#endif

// Pedestal
#ifndef PEDESTAL_VIEW_HH
#include "Pedestal/View.hh"
#endif


namespace Pedestal
{
	
	class PushButton : public View
	{
		private:
			ControlRef  itsControl;
		
		public:
			PushButton() : itsControl()
			{
			}
			
			virtual Nitrogen::Str255 Title() const = 0;
			
			virtual const void* RefCon() const = 0;
			
			ControlRef Get() const  { return itsControl; }
			
			void Install( const Rect& bounds );
			
			void Uninstall();
			
			void SetBounds( const Rect& bounds );
			
			void Activate( bool activating );
	};
	
}

#endif

