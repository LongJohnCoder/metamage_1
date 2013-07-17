/*
	gui/new/icon.cc
	---------------
*/

#include "Genie/FS/gui/new/icon.hh"

// plus
#include "plus/serialize.hh"

// Nitrogen
#include "Nitrogen/Quickdraw.hh"

// ClassicToolbox
#include "ClassicToolbox/MacWindows.hh"

// Pedestal
#include "Pedestal/View.hh"

// vfs
#include "vfs/node/types/fixed_dir.hh"

// Genie
#include "Genie/FS/FSTree.hh"
#include "Genie/FS/FSTree_Property.hh"
#include "Genie/FS/Icon_data.hh"
#include "Genie/FS/Views.hh"
#include "Genie/Utilities/simple_map.hh"


namespace Nitrogen
{
	
	static const IconTransformType kTransformDisabled = IconTransformType( ::kTransformDisabled );
	
}

namespace Genie
{
	
	namespace N = Nitrogen;
	namespace Ped = Pedestal;
	
	
	struct Icon_Parameters
	{
		boost::intrusive_ptr< IconData >  data;
		Nitrogen::IconAlignmentType       align;
		Nitrogen::IconTransformType       xform;
		char                              label;
		bool                              selected;
		bool                              disabling;
		
		Icon_Parameters() : align(), xform(), label(), selected(), disabling()
		{
		}
	};
	
	typedef simple_map< const FSTree*, Icon_Parameters > IconMap;
	
	static IconMap gIconMap;
	
	
	class Icon : public Ped::View
	{
		private:
			typedef const FSTree* Key;
			
			Key itsKey;
		
		public:
			Icon( Key key ) : itsKey( key )
			{
			}
			
			void Draw( const Rect& bounds, bool erasing );
			
			void Activate( bool activating );
	};
	
	static inline Nitrogen::IconTransformType
	//
	CombinedIconTransforms( const Icon_Parameters& params )
	{
		typedef Nitrogen::IconTransformType Type;
		
		Type state    = Type( params.xform                         );
		Type label    = Type( params.label    * kTransformLabel1   );
		Type selected = Type( params.selected * kTransformSelected );
		
		return state | label | selected;
	}
	
	void Icon::Draw( const Rect& bounds, bool erasing )
	{
		if ( erasing )
		{
			N::EraseRect( bounds );
		}
		
		Icon_Parameters& params = gIconMap[ itsKey ];
		
		if ( params.data.get() )
		{
			params.data->Plot( bounds,
			                   params.align,
			                   CombinedIconTransforms( params ) );
		}
	}
	
	void Icon::Activate( bool activating )
	{
		Icon_Parameters& params = gIconMap[ itsKey ];
		
		if ( params.disabling )
		{
			params.xform = activating ? N::kTransformNone
			                          : N::kTransformDisabled;
			
			N::InvalRect( N::GetPortBounds( N::GetQDGlobalsThePort() ) );
		}
	}
	
	
	static boost::intrusive_ptr< Ped::View > CreateView( const FSTree* delegate )
	{
		return new Icon( delegate );
	}
	
	
	static void DestroyDelegate( const FSTree* delegate )
	{
		gIconMap.erase( delegate );
	}
	
	
	namespace
	{
		
		N::IconAlignmentType& Alignment( const FSTree* view )
		{
			return gIconMap[ view ].align;
		}
		
		N::IconTransformType& Transform( const FSTree* view )
		{
			return gIconMap[ view ].xform;
		}
		
		char& Label( const FSTree* view )
		{
			return gIconMap[ view ].label;
		}
		
		bool& Selected( const FSTree* view )
		{
			return gIconMap[ view ].selected;
		}
		
		bool& Disabling( const FSTree* view )
		{
			return gIconMap[ view ].disabling;
		}
		
	}
	
	static FSTreePtr Data_Factory( const FSTree*        parent,
	                               const plus::string&  name,
	                               const void*          args )
	{
		boost::intrusive_ptr< IconData >& data = gIconMap[ parent ].data;
		
		if ( data.get() == NULL )
		{
			data = boost::intrusive_ptr< IconData >( new IconData );
		}
		
		return New_FSTree_Icon_data( parent, name, data );
	}
	
	
	#define PROPERTY( prop )  &new_property, &property_params_factory< prop >::value
	
	typedef View_Property< plus::serialize_unsigned< N::IconAlignmentType >, Alignment >  Alignment_Property;
	typedef View_Property< plus::serialize_unsigned< N::IconTransformType >, Transform >  Transform_Property;
	
	typedef View_Property< plus::serialize_unsigned< char >, Label >  Label_Property;
	
	typedef View_Property< plus::serialize_bool, Selected  >  Selected_Property;
	typedef View_Property< plus::serialize_bool, Disabling >  Disabling_Property;
	
	static const vfs::fixed_mapping local_mappings[] =
	{
		{ "data", &Data_Factory },
		
		{ "align",     PROPERTY( Alignment_Property ) },
		{ "xform",     PROPERTY( Transform_Property ) },
		{ "label",     PROPERTY( Label_Property     ) },
		{ "selected",  PROPERTY( Selected_Property  ) },
		{ "disabling", PROPERTY( Disabling_Property ) },
		
		{ NULL, NULL }
	};
	
	FSTreePtr New_FSTree_new_icon( const FSTree*        parent,
	                               const plus::string&  name,
	                               const void*          args )
	{
		return New_new_view( parent,
		                     name,
		                     &CreateView,
		                     local_mappings,
		                     &DestroyDelegate );
	}
	
}

