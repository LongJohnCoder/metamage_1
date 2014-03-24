// Carbonate/MacWindows.cc


#include <LowMem.h>
#include <MacWindows.h>

#if ACCESSOR_CALLS_ARE_FUNCTIONS
// Compile the Carbon accessors as extern pascal functions.
#define CARBONATE_LINKAGE pascal
//#include "Carbonate/MacWindows.hh"
#endif


#if !ACCESSOR_CALLS_ARE_FUNCTIONS

pascal WindowRef GetWindowList()
{
	return LMGetWindowList();
}

#endif

OSStatus SetWindowAlpha( WindowRef window, float alpha )
{
	return unimpErr;
}

OSStatus GetWindowAlpha( WindowRef window, float* outAlpha )
{
	if ( outAlpha != NULL )
	{
		*outAlpha = 1.0;
	}
	
	return noErr;
}

