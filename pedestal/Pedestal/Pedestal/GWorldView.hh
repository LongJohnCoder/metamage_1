/*	=============
 *	GWorldView.hh
 *	=============
 */

#ifndef PEDESTAL_GWORLDVIEW_HH
#define PEDESTAL_GWORLDVIEW_HH

// Nitrogen
#include "Nitrogen/QDOffScreen.h"

// Pedestal
#include "Pedestal/View.hh"


namespace Pedestal
{
	
	class GWorldView : public View
	{
		private:
			Rect                                   itsBounds;
			Nucleus::Owned< Nitrogen::GWorldPtr >  itsGWorld;
		
		public:
			GWorldView( const Rect& bounds );
			
			Nitrogen::GWorldPtr Get() const  { return itsGWorld.Get(); }
			
			const Rect& Bounds() const  { return itsBounds; }
			
			void Erase( const Rect& bounds );
			
			void Resize( short width, short height );
			
			void Draw( const Rect& bounds, bool erasing );
	};
	
}

#endif

