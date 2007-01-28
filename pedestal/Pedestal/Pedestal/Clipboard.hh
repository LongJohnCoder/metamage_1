/*	============
 *	Clipboard.hh
 *	============
 */

#ifndef PEDESTAL_CLIPBOARD_HH
#define PEDESTAL_CLIPBOARD_HH

// Mac OS
#ifndef __TEXTEDIT__
#include <TextEdit.h>
#endif


namespace Pedestal
{
	
	struct Clipboard
	{
		Clipboard();
		~Clipboard();
		
		static void Suspend();
		static void Resume ();
		
		static void TECut  ( TEHandle hTE );
		static void TECopy ( TEHandle hTE );
		static void TEPaste( TEHandle hTE );
	};
	
}

#endif

