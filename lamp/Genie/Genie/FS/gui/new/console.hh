/*
	gui/new/console.hh
	------------------
*/

#ifndef GENIE_FS_GUI_NEW_CONSOLE_HH
#define GENIE_FS_GUI_NEW_CONSOLE_HH

// Genie
#include "Genie/FS/FSTree.hh"


namespace Genie
{
	
	FSTreePtr New_FSTree_new_console( const FSTreePtr&     parent,
	                                  const plus::string&  name,
	                                  const void*          args );
	
}

#endif

