/*
	SetPort_GetWindow.cc
	--------------------
	
	Copyright 2009-2010, Joshua Juran
*/

#include "Pedestal/SetPort_GetWindow.hh"

// Nitrogen
#include "Nitrogen/MacWindows.hh"

// Pedestal
#include "Pedestal/Window.hh"


namespace Pedestal
{
	
	namespace N = Nitrogen;
	
	
	Window* SetPort_GetWindow( WindowRef windowRef )
	{
		if ( windowRef != NULL )
		{
			if ( N::GetWindowKind( windowRef ) == N::kApplicationWindowKind )
			{
				if ( Window* window = get_window_owner( windowRef ) )
				{
					N::SetPortWindowPort( windowRef );
					
					return window;
				}
			}
		}
		
		return NULL;
	}
	
}
