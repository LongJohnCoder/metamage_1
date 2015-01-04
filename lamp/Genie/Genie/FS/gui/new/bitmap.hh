/*
	gui/new/bitmap.hh
	-----------------
*/

#ifndef GENIE_FS_GUI_NEW_BITMAP_HH
#define GENIE_FS_GUI_NEW_BITMAP_HH

// plus
#include "plus/string.hh"

// Genie
#include "Genie/FS/FSTreePtr.hh"


namespace Genie
{
	
	FSTreePtr New_FSTree_new_bitmap( const FSTree*        parent,
	                                 const plus::string&  name,
	                                 const void* );
	
}

#endif
