/*
	Genie/FS/new/port.hh
	--------------------
*/

#ifndef GENIE_FS_NEW_PORT_HH
#define GENIE_FS_NEW_PORT_HH

// Genie
#include "Genie/FS/FSTree.hh"


namespace Genie
{
	
	class FSTree_new_port : public FSTree
	{
		public:
			FSTree_new_port( const FSTreePtr&     parent,
			                 const plus::string&  name );
			
			bool IsDirectory() const  { return true; }
			
			boost::shared_ptr< IOHandle > ChangeToDirectory() const;
	};
	
}

#endif

