/*
	gui/new/stack.cc
	----------------
*/

#include "Genie/FS/gui/new/stack.hh"

// Standard C++
#include <vector>

// POSIX
#include <sys/stat.h>

// plus
#include "plus/simple_map.hh"

// Pedestal
#include "Pedestal/Stack.hh"

// vfs
#include "vfs/dir_contents.hh"
#include "vfs/dir_entry.hh"
#include "vfs/node.hh"
#include "vfs/methods/dir_method_set.hh"
#include "vfs/methods/item_method_set.hh"
#include "vfs/methods/node_method_set.hh"
#include "vfs/node/types/property_file.hh"
#include "vfs/primitives/remove.hh"

// Genie
#include "Genie/FS/Views.hh"


/*
	cd /gui/new/port
	ln /gui/new/stack view
	ln /gui/new/color v/background/view
	ln /gui/new/frame v/frame/view
	ls v
		frame
		background
	
*/

namespace Genie
{
	
	namespace Ped = Pedestal;
	
	
	struct Named_Subview
	{
		plus::string                       name;
		boost::intrusive_ptr< Ped::View >  view;
		const vfs::node*                   node;
		
		Named_Subview() : node()
		{
		}
		
		Named_Subview( const plus::string& n ) : name( n ), node()
		{
		}
	};
	
	typedef std::vector< Named_Subview > ViewList;
	
	struct Stack_Parameters
	{
		ViewList  v;
		
		Stack_Parameters()
		{
		}
	};
	
	static Named_Subview*
	//
	find_subview( Stack_Parameters& params, const plus::string& name )
	{
		ViewList& v = params.v;
		
		for ( ViewList::iterator it = v.begin();  it != v.end();  ++it )
		{
			if ( it->name == name )
			{
				return &*it;
			}
		}
		
		return NULL;
	}
	
	static Named_Subview&
	//
	find_or_append_subview( Stack_Parameters& params, const plus::string& name )
	{
		ViewList& v = params.v;
		
		if ( Named_Subview* subview = find_subview( params, name ) )
		{
			return *subview;
		}
		
		v.push_back( Named_Subview( name ) );
		
		return v.back();
	}
	
	typedef plus::simple_map< const vfs::node*, Stack_Parameters > Stack_Parameters_Map;
	
	static Stack_Parameters_Map gStack_Parameters_Map;
	
	
	class Stack : public Ped::Stack
	{
		private:
			typedef const vfs::node* Key;
			
			Key itsKey;
		
		public:
			Stack( Key key ) : itsKey( key )
			{
			}
			
			unsigned ViewCount() const  { return gStack_Parameters_Map[ itsKey ].v.size(); }
			
			View& GetNthView( unsigned n )  { return *gStack_Parameters_Map[ itsKey ].v.at( n - 1 ).view; }
	};
	
	
	static boost::intrusive_ptr< Ped::View > StackFactory( const vfs::node* delegate )
	{
		return new Stack( delegate );
	}
	
	
	static
	Ped::View* get_subview( const vfs::node* layer, const plus::string& name )
	{
		Stack_Parameters& params = gStack_Parameters_Map[ layer->owner() ];
		
		return find_or_append_subview( params, layer->name() ).view.get();
	}
	
	static
	void set_subview( const vfs::node*     layer,
	                  const plus::string&  name,
	                  Ped::View*           view )
	{
		Stack_Parameters& params = gStack_Parameters_Map[ layer->owner() ];
		
		find_or_append_subview( params, layer->name() ).view = view;
	}
	
	const View_Accessors access =
	{
		&get_subview,
		&set_subview,
	};
	
	static void destroy_layer( const vfs::node* that )
	{
		Stack_Parameters& params = gStack_Parameters_Map[ that->owner() ];
		
		Named_Subview* subview = find_subview( params, that->name() );
		
		if ( subview == NULL )
		{
			return;
		}
		
		ViewList& v = params.v;
		
		v.erase( v.begin() + (subview - &v[0]) );
	}
	
	
	static vfs::node_ptr stack_subview_factory( const vfs::node*     parent,
	                                            const plus::string&  name,
	                                            const void*          args )
	{
		return New_View( parent, name, access );
	}
	
	static const vfs::fixed_mapping stack_mappings[] =
	{
		{ "view", &stack_subview_factory },
		{ "v",    &new_view_dir          },
		
		{ NULL, NULL }
	};
	
	
	static void stack_remove( const vfs::node* that )
	{
		Stack_Parameters& params = gStack_Parameters_Map[ that ];
		
		while ( !params.v.empty() )
		{
			const vfs::node* layer = params.v.back().node;
			
			remove( *layer );
			
			RemoveAllViewParameters( layer );
		}
		
		gStack_Parameters_Map.erase( that );
	}
	
	static vfs::node_ptr stack_lookup( const vfs::node*     that,
	                                   const plus::string&  name,
	                                   const vfs::node*     parent )
	{
		Named_Subview& layer = find_or_append_subview( gStack_Parameters_Map[ that ], name );
		
		if ( layer.node == NULL )
		{
			vfs::node_ptr layer_node = vfs::fixed_dir( parent,
			                                           name,
			                                           stack_mappings,
			                                           &destroy_layer );
			
			layer.node = layer_node.get();
			
			return layer_node;
		}
		
		return layer.node;
	}
	
	
	static void stack_listdir( const vfs::node* that, vfs::dir_contents& cache )
	{
		typedef ViewList Sequence;
		
		const Sequence& sequence = gStack_Parameters_Map[ that ].v;
		
		typedef Sequence::const_iterator Iter;
		
		const Iter end = sequence.end();
		
		for ( Iter it = sequence.begin();  it != end;  ++it )
		{
			const ino_t inode = 0;
			
			cache.push_back( vfs::dir_entry( inode, it->name ) );
		}
	}
	
	static const vfs::item_method_set stack_item_methods =
	{
		NULL,
		NULL,
		NULL,
		NULL,
		&stack_remove,
	};
	
	static const vfs::dir_method_set stack_dir_methods =
	{
		&stack_lookup,
		&stack_listdir
	};
	
	static const vfs::node_method_set stack_methods =
	{
		&stack_item_methods,
		NULL,
		NULL,
		&stack_dir_methods
	};
	
	
	static vfs::node_ptr create_delegate_for_new_stack( const vfs::node*     that,
	                                                    const vfs::node*     parent,
	                                                    const plus::string&  name )
	{
		return new vfs::node( parent, name, S_IFDIR | 0700, &stack_methods );
	}
	
	vfs::node_ptr New_stack( const vfs::node*     parent,
	                         const plus::string&  name,
	                         const void*          args )
	{
		return New_new_view( parent,
		                     name,
		                     &StackFactory,
		                     NULL,
		                     &stack_remove,
		                     0,
		                     &create_delegate_for_new_stack );
	}
	
}
