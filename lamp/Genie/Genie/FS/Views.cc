/*	========
 *	Views.cc
 *	========
 */

#include "Genie/FS/Views.hh"

// Mac OS X
#ifdef __APPLE__
#include <CoreServices/CoreServices.h>
#endif

#ifndef __MACTYPES__
#include <MacTypes.h>
#endif

// POSIX
#include <sys/stat.h>

// iota
#include "iota/swap.hh"

// Debug
#include "debug/assert.hh"

// poseven
#include "poseven/types/errno_t.hh"

// PEdestal
#include "Pedestal/EmptyView.hh"

// vfs
#include "vfs/node.hh"
#include "vfs/methods/dir_method_set.hh"
#include "vfs/methods/file_method_set.hh"
#include "vfs/methods/item_method_set.hh"
#include "vfs/methods/node_method_set.hh"
#include "vfs/node/types/null.hh"
#include "vfs/primitives/listdir.hh"
#include "vfs/primitives/lookup.hh"
#include "vfs/primitives/mkdir.hh"
#include "vfs/primitives/remove.hh"

// Genie
#include "Genie/FS/gui/port/ADDR.hh"
#include "Genie/Utilities/simple_map.hh"


namespace Genie
{
	
	namespace p7 = poseven;
	namespace Ped = Pedestal;
	
	
	struct ViewParameters
	{
		vfs::node_ptr     itsDelegate;
		ViewFactory       itsFactory;
		const vfs::node*  itsWindowKey;
		
		ViewParameters() : itsFactory(), itsWindowKey()
		{
		}
		
		void swap( ViewParameters& other );
	};
	
	void ViewParameters::swap( ViewParameters& other )
	{
		itsDelegate.swap( other.itsDelegate );
		
		using iota::swap;
		
		swap( itsFactory,   other.itsFactory   );
		swap( itsWindowKey, other.itsWindowKey );
	}
	
	typedef simple_map< const FSTree*, ViewParameters > ViewParametersMap;
	
	static ViewParametersMap gViewParametersMap;
	
	
	static inline const ViewParameters* find_view( const FSTree* parent )
	{
		return gViewParametersMap.find( parent );
	}
	
	static bool view_exists( const FSTree* parent )
	{
		return find_view( parent ) != NULL;
	}
	
	static void add_view_parameters( const FSTree*     parent,
	                                 const vfs::node&  delegate,
	                                 ViewFactory       factory )
	{
		ViewParameters& params = gViewParametersMap[ parent ];
		
		params.itsFactory  = factory;
		params.itsDelegate = &delegate;
	}
	
	static void add_view_port_key( const FSTree* parent, const FSTree* windowKey )
	{
		ASSERT( find_view( parent ) != NULL );
		
		ASSERT( find_view( parent )->itsDelegate.get() != NULL );
		
		gViewParametersMap[ parent ].itsWindowKey = windowKey;
	}
	
	static void DeleteDelegate( vfs::node_ptr& delegate_ref )
	{
		if ( const FSTree* delegate = delegate_ref.get() )
		{
			vfs::node_ptr delegate_copy;
			
			delegate_copy.swap( delegate_ref );
			
			try
			{
				remove( *delegate );
			}
			catch ( ... )
			{
				if ( TARGET_CONFIG_DEBUGGING )
				{
					// This might happen in __destroy_global_chain(),
					// so don't ASSERT which relies on trashed infrastructure.
					
					::DebugStr( "\p" "remove( delegate ) may not throw" );
				}
			}
			
			RemoveAllViewParameters( delegate );
		}
	}
	
	void RemoveAllViewParameters( const FSTree* parent )
	{
		if ( ViewParameters* it = gViewParametersMap.find( parent ) )
		{
			ViewParameters temp;
			
			temp.swap( *it );
			
			gViewParametersMap.erase( parent );
			
			notify_port_of_view_loss( temp.itsWindowKey, temp.itsDelegate.get() );
			
			DeleteDelegate( temp.itsDelegate );
		}
	}
	
	static boost::intrusive_ptr< Ped::View > make_view( const FSTree* parent )
	{
		if ( const ViewParameters* params = find_view( parent ) )
		{
			const FSTree* delegate = params->itsDelegate.get();
			
			ViewFactory factory = params->itsFactory;
			
			ASSERT( delegate != NULL );
			ASSERT( factory  != NULL );
			
			return factory( delegate );
		}
		
		return boost::intrusive_ptr< Ped::View >();
	}
	
	static const vfs::node_ptr& GetViewDelegate( const FSTree* view )
	{
		const ViewParameters* params = find_view( view->owner() );
		
		if ( params == NULL )
		{
			p7::throw_errno( ENOENT );
		}
		
		return params->itsDelegate;
	}
	
	const FSTree* GetViewWindowKey( const FSTree* view )
	{
		const ViewParameters* params = find_view( view->owner() );
		
		return params ? params->itsWindowKey : NULL;
	}
	
	
	bool InvalidateWindowForView( const FSTree* view )
	{
		const FSTree* windowKey = GetViewWindowKey( view );
		
		return invalidate_port_WindowRef( windowKey );
	}
	
	
	static void new_view_hardlink( const FSTree*  that,
	                               const FSTree*  dest );
	
	static const vfs::file_method_set new_view_file_methods =
	{
		NULL,
		NULL,
		&new_view_hardlink
	};
	
	static const vfs::node_method_set new_view_methods =
	{
		NULL,
		NULL,
		NULL,
		NULL,
		&new_view_file_methods
	};
	
	struct new_view_extra
	{
		vfs::fixed_mapping const*  mappings;
		vfs::node_destructor       destructor;
		ViewFactory                view_factory;
		DelegateFactory            delegate_factory;
		size_t                     extra_annex_size;
	};
	
	vfs::node_ptr New_new_view( const vfs::node*           parent,
	                            const plus::string&        name,
	                            ViewFactory                factory,
	                            const vfs::fixed_mapping*  mappings,
	                            vfs::node_destructor       dtor,
	                            size_t                     extra_annex_size,
	                            DelegateFactory            delegate_factory )
	
	{
		FSTree* result = new FSTree( parent,
		                             name,
		                             S_IFREG | 0,
		                             &new_view_methods,
		                             sizeof (new_view_extra) );
		
		new_view_extra& extra = *(new_view_extra*) result->extra();
		
		extra.mappings         = mappings;
		extra.destructor       = dtor;
		extra.view_factory     = factory;
		extra.delegate_factory = delegate_factory;
		extra.extra_annex_size = extra_annex_size;
		
		return result;
	}
	
	vfs::node_ptr create_default_delegate_for_new_view( const vfs::node*     that,
	                                                    const vfs::node*     parent,
	                                                    const plus::string&  name )
	{
		new_view_extra& extra = *(new_view_extra*) that->extra();
		
		vfs::node_ptr delegate = fixed_dir( parent, name, extra.mappings, extra.destructor );
		
		return delegate;
	}
	
	static void new_view_hardlink( const FSTree*  that,
	                               const FSTree*  target )
	{
		new_view_extra& extra = *(new_view_extra*) that->extra();
		
		const FSTree* parent = target->owner();
		
		const FSTree* key = parent;
		
		vfs::node_ptr delegate = extra.delegate_factory( that, parent, "v" );
		
		add_view_parameters( key, *delegate, extra.view_factory );
		
		mkdir( *target, 0 );  // mode is ignored
	}
	
	
	struct view_extra
	{
		ViewGetter  get;
		ViewPurger  purge;
	};
	
	
	static boost::intrusive_ptr< Pedestal::View >& view_of( const FSTree* that )
	{
		ASSERT( that != NULL );
		
		const view_extra& extra = *(view_extra*) that->extra();
		
		return extra.get( that->owner(), that->name() );
	}
	
	static void unview_mkdir( const FSTree* that, mode_t mode )
	{
		const FSTree* parent = that->owner();
		
		const plus::string& name = that->name();
		
		const FSTree* windowKey = GetViewWindowKey( parent );
		
		if ( windowKey == NULL )
		{
			ASSERT( parent != NULL );
			
			windowKey = GetViewWindowKey( parent->owner() );
		}
		
		if ( windowKey == NULL )
		{
			windowKey = parent;
		}
		
		add_view_port_key( parent, windowKey );
		
		boost::intrusive_ptr< Ped::View > view = make_view( parent );
		
		view_of( that ) = view;
		
		// Install and invalidate if window exists
		install_view_in_port( view, windowKey );
	}
	
	static const vfs::dir_method_set unview_dir_methods =
	{
		NULL,
		NULL,
		&unview_mkdir
	};
	
	static const vfs::node_method_set unview_methods =
	{
		NULL,
		NULL,
		NULL,
		&unview_dir_methods
	};
	
	
	static void view_touch( const FSTree* that )
	{
		InvalidateWindowForView( that );
	}
	
	static void view_remove( const FSTree* that )
	{
		const view_extra& extra = *(view_extra*) that->extra();
		
		const FSTree* parent = that->owner();
		
		const plus::string& name = that->name();
		
		const FSTree* windowKey = GetViewWindowKey( that );
		
		uninstall_view_from_port( view_of( that ), windowKey );
		
		view_of( that ) = Ped::EmptyView::Get();
		
		RemoveAllViewParameters( parent );
		
		if ( extra.purge )
		{
			extra.purge( parent, name );
		}
	}
	
	static vfs::node_ptr view_lookup( const vfs::node*     that,
	                                  const plus::string&  name,
	                                  const vfs::node*     parent )
	{
		const plus::string& real_name = name.empty() ? plus::string( "." ) : name;
		
		return lookup( *GetViewDelegate( that ), real_name, NULL );
	}
	
	static void view_listdir( const FSTree*       that,
	                          vfs::dir_contents&  cache )
	{
		listdir( *GetViewDelegate( that ), cache );
	}
	
	static const vfs::item_method_set view_item_methods =
	{
		NULL,
		NULL,
		&view_touch,
		NULL,
		&view_remove,
	};
	
	static const vfs::dir_method_set view_dir_methods =
	{
		&view_lookup,
		&view_listdir
	};
	
	static const vfs::node_method_set view_methods =
	{
		&view_item_methods,
		NULL,
		NULL,
		&view_dir_methods
	};
	
	static const vfs::node_method_set viewdir_methods =
	{
		NULL,
		NULL,
		NULL,
		&view_dir_methods
	};
	
	vfs::node_ptr New_View( const vfs::node*     parent,
	                        const plus::string&  name,
	                        ViewGetter           get,
	                        ViewPurger           purge )
	{
		const bool exists = view_exists( parent );
		
		const mode_t mode = !exists ? 0
		                  : name.size() == 1 ? S_IFDIR | 0700
		                  :                    S_IFREG | 0600;
		
		const vfs::node_method_set& methods = exists ? view_methods
		                                             : unview_methods;
		
		FSTree* result = new FSTree( parent,
		                             name,
		                             mode,
		                             &methods,
		                             sizeof (view_extra) );
		
		view_extra& extra = *(view_extra*) result->extra();
		
		extra.get   = get;
		extra.purge = purge;
		
		return result;
	}
	
	vfs::node_ptr new_view_dir( const vfs::node*     parent,
	                            const plus::string&  name,
	                            const void*          /* args */ )
	{
		if ( !view_exists( parent ) )
		{
			return vfs::null();
		}
		
		vfs::node* result = new vfs::node( parent,
		                                   name,
		                                   S_IFDIR | 0700,
		                                   &viewdir_methods );
		
		return result;
	}
	
}
