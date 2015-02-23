/*	=========
 *	FSSpec.hh
 *	=========
 */

#ifndef GENIE_FILESYSTEM_FSSPEC_HH
#define GENIE_FILESYSTEM_FSSPEC_HH

// Nitrogen
#ifndef MAC_FILES_TYPES_FSDIRSPEC_HH
#include "Mac/Files/Types/FSDirSpec.hh"
#endif

// vfs
#include "vfs/node_ptr.hh"

// Files.h
struct FSSpec;


namespace Genie
{
	
	Mac::FSDirSpec Dir_From_CInfo( const CInfoPBRec& cInfo );
	
	Mac::FSDirSpec Dir_From_FSSpec( const FSSpec& dir );
	
	vfs::node_ptr FSTreeFromFSSpec( const FSSpec& item );
	
	vfs::node_ptr node_from_dirID( short vRefNum, long dirID );
	
	template < class Dir >
	inline vfs::node_ptr FSTreeFromFSDirSpec( const Dir& dir )
	{
		return node_from_dirID( dir.vRefNum, dir.dirID );
	}
	
}

#endif
