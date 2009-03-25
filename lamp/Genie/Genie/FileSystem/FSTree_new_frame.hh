/*	===================
 *	FSTree_new_frame.hh
 *	===================
 */

#ifndef GENIE_FILESYSTEM_FSTREE_NEW_FRAME_HH
#define GENIE_FILESYSTEM_FSTREE_NEW_FRAME_HH

// Genie
#include "Genie/FileSystem/FSTree.hh"


namespace Genie
{
	
	FSTreePtr New_FSTree_new_frame( const FSTreePtr& parent, const std::string& name );
	
}

#endif

