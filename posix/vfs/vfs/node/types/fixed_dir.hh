/*
	fixed_dir.hh
	------------
*/

#ifndef VFS_NODE_TYPES_FIXEDDIR_HH
#define VFS_NODE_TYPES_FIXEDDIR_HH

// plus
#include "plus/string.hh"

// vfs
#include "vfs/node_ptr.hh"


namespace vfs
{
	
	typedef void (*node_destructor)( const node* );
	
	
	typedef node_ptr (*node_factory)( const node*,
	                                  const plus::string&,
	                                  const void* );
	
	struct fixed_mapping
	{
		const char*   name;
		node_factory  f;
		const void*   args;
	};
	
	extern const fixed_mapping empty_mappings[];
	
	
	struct fixed_dir_extra
	{
		fixed_mapping const*  mappings;
	};
	
	node_ptr fixed_dir( uid_t                  user,
	                    const fixed_mapping    mappings[] = empty_mappings,
	                    void                 (*dtor)(const node*) = NULL,
	                    size_t                 extra_annex_size = 0 );
	
	node_ptr fixed_dir( const node*            parent,
	                    const plus::string&    name,
	                    const fixed_mapping    mappings[] = empty_mappings,
	                    void                 (*dtor)(const node*) = NULL,
	                    size_t                 extra_annex_size = 0 );
	
	node_ptr fixed_dir_factory( const node*          parent,
	                            const plus::string&  name,
	                            const void*          args );
	
}

#endif
