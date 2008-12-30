/*	=================
 *	TrackScrollbar.cc
 *	=================
 */

#include "Pedestal/TrackScrollbar.hh"

// Mac OS
#include <ControlDefinitions.h>

// Standard C++
#include <algorithm>

// Nucleus
#include "Nucleus/NAssert.h"
#include "Nucleus/Saved.h"

// Pedestal
#include "Pedestal/Scroller_beta.hh"


namespace Pedestal
{
	
	namespace N = Nitrogen;
	namespace NN = Nucleus;
	
	
	static inline long Gestalt_AppearanceManager()
	{
		return N::Gestalt( N::Gestalt_Selector( gestaltAppearanceAttr ), 0 );
	}
	
#if TARGET_API_MAC_CARBON
	
	static inline bool Has_AppearanceManager()  { return true; }
	
#else
	
	static bool Has_AppearanceManager()
	{
		static bool exists = Gestalt_AppearanceManager() != 0;
		
		return exists;
	}
	
#endif
	
	static short SetClippedControlValue( ControlRef control, short value )
	{
		value = std::max( value, N::GetControlMinimum( control ) );
		value = std::min( value, N::GetControlMaximum( control ) );
		
		N::SetControlValue( control, value );
		
		return value;
	}
	
	static inline Scrollbar_UserData* GetUserDataFromScrollbar( ControlRef control )
	{
		Scrollbar_UserData* userData = N::GetControlReference( control );
		
		ASSERT( userData != NULL );
		
		return userData;
	}
	
	static inline ScrollerAPI* RecoverScrollerFromScrollbar( ControlRef control )
	{
		Scrollbar_UserData* userData = GetUserDataFromScrollbar( control );
		
		ASSERT( userData != NULL );
		
		ASSERT( userData->fetchHook != NULL );
		
		ScrollerAPI* scroller = userData->fetchHook( control );
		
		ASSERT( scroller != NULL );
		
		return scroller;
	}
	
	static inline void SetScrollOffset( ScrollerAPI* scroller, bool vertical, short value )
	{
		ASSERT( scroller != NULL );
		
		if ( vertical )
		{
			scroller->SetVOffset( value );
		}
		else
		{
			scroller->SetHOffset( value );
		}
	}
	
	namespace
	{
		
		void ScrollbarAction( ControlRef control, N::ControlPartCode part )
		{
			const short value = N::GetControlValue( control );
			
			const bool vertical = GetUserDataFromScrollbar( control )->vertical;
			
			const bool dragged = part == N::kControlIndicatorPart;
			
			ScrollerAPI* scroller = RecoverScrollerFromScrollbar( control );
			
			short oldValue = value;
			short newValue = value;
			
			if ( dragged )
			{
				oldValue = vertical ? scroller->GetVOffset()
				                    : scroller->GetHOffset();
				
				newValue = value;
			}
			else
			{
				oldValue = value;
				
				short scrollDistance = 1;
				
				if ( part == N::kControlPageUpPart  ||  part == N::kControlPageDownPart )
				{
					const short viewLength = vertical ? scroller->ViewHeight()
					                                  : scroller->ViewWidth ();
					
					const short pageDistance = viewLength - 1;
					
					scrollDistance = pageDistance;
				}
				
				if ( part == N::kControlUpButtonPart  ||  part == N::kControlPageUpPart )
				{
					scrollDistance = -scrollDistance;
				}
				
				newValue = SetClippedControlValue( control, value + scrollDistance );
			}
			
			if ( newValue != oldValue )
			{
				SetScrollOffset( scroller, vertical, newValue );
			}
		}
		
	}
	
	
	void TrackScrollbar( ControlRef control, N::ControlPartCode part, Point point )
	{
		NN::Saved< N::Clip_Value > savedClip;
		
		N::ClipRect( N::GetPortBounds( N::GetQDGlobalsThePort() ) );
		
		const Scrollbar_UserData* userData = GetUserDataFromScrollbar( control );
		
		ASSERT( userData != NULL );
		
		// Get the current scrollbar value.
		const short oldValue = N::GetControlValue( control );
		
		switch ( part )
		{
			case kControlIndicatorPart:
				// The user clicked on the indicator
				
				if ( !Has_AppearanceManager() )
				{
					// Classic scrolling, handled specially.
					
					// Let the system track the drag...
					part = N::TrackControl( control, point );
					
					if ( part == N::kControlIndicatorPart )
					{
						// Drag was successful (i.e. within bounds).
						const short newValue = N::GetControlValue( control );
						
						if ( newValue != oldValue )
						{
							ScrollerAPI* scroller = RecoverScrollerFromScrollbar( control );
							
							ASSERT( scroller != NULL );
							
							SetScrollOffset( scroller, userData->vertical, newValue );
						}
					}
					
					// Break here for classic thumb-scrolling (whether successful or not).
					break;
				}
				// else fall through for live feedback scrolling
			case kControlUpButtonPart:
			case kControlDownButtonPart:
			case kControlPageUpPart:
			case kControlPageDownPart:
				if (( part = N::TrackControl< ScrollbarAction >( control, point ) ))
				{
					if ( userData->afterHook  &&  N::GetControlValue( control ) != oldValue )
					{
						userData->afterHook( control );
					}
				}
				
				break;
			
			default:
				break;
		}
	}
	
}

