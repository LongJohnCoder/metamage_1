/*
	ResolvableSymLink.hh
	--------------------
*/

#ifndef GENIE_FS_RESOLVABLESYMLINK_HH
#define GENIE_FS_RESOLVABLESYMLINK_HH

// Genie
#include "Genie/FS/FSTree.hh"


namespace Genie
{
	
	class FSTree_ResolvableSymLink : public FSTree
	{
		public:
			FSTree_ResolvableSymLink( const FSTreePtr&     parent,
			                          const plus::string&  name );
			
			plus::string ReadLink() const;
	};
	
}

#endif

