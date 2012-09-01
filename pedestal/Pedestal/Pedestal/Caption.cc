/*	==========
 *	Caption.cc
 *	==========
 */

#include "Pedestal/Caption.hh"

// missing-macos
#ifdef MAC_OS_X_VERSION_10_7
#ifndef MISSING_QUICKDRAWTEXT_H
#include "missing/QuickdrawText.h"
#endif
#endif

// nucleus
#include "nucleus/saved.hh"

// Nitrogen
#include "Nitrogen/Quickdraw.hh"
#include "Nitrogen/TextEdit.hh"


namespace Nitrogen
{
	
	class TextMode_Setting
	{
		public:
			typedef short  value_type;
			typedef short  param_type;
			
			value_type get() const  { return GetPortTextMode( GetQDGlobalsThePort() ); }
			
			void set( param_type mode ) const  { TextMode( mode ); }
	};
	
}

namespace Pedestal
{
	
	namespace n = nucleus;
	namespace N = Nitrogen;
	
	
	void Caption::Draw( const Rect& bounds, bool erasing )
	{
		n::saved< N::Clip > savedClip;
		
		Rect newBounds = bounds;
		
		if ( !Wrapped() )
		{
			newBounds.right = 30000;
			
			N::ClipRect( bounds );
		}
		
		n::saved< N::TextMode_Setting > savedTextMode;
		
		if ( Disabled() )
		{
			::TextMode( grayishTextOr );
		}
		
		N::TETextBox( Text(), newBounds );
	}
	
}

