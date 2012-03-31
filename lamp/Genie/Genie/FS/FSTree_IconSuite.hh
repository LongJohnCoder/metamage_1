/*	===================
 *	FSTree_IconSuite.hh
 *	===================
 */

#ifndef GENIE_FILESYSTEM_FSTREE_ICONSUITE_HH
#define GENIE_FILESYSTEM_FSTREE_ICONSUITE_HH

// plus
#include "plus/string.hh"

// nucleus
#ifndef NUCLEUS_SHARED_HH
#include "nucleus/shared.hh"
#endif

// Nitrogen
#ifndef NITROGEN_ICONS_HH
#include "Nitrogen/Icons.hh"
#endif

// Genie
#include "Genie/FS/FSTreePtr.hh"


namespace Genie
{
	
	FSTreePtr
	//
	New_FSTree_IconSuite( const FSTreePtr&                                  parent,
			              const plus::string&                               name,
			              const nucleus::shared< Nitrogen::IconSuiteRef >&  iconSuite );
	
	const Nitrogen::IconSuiteRef Fetch_IconSuite();
	
}

#endif

