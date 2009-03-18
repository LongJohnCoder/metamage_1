/*	======================
 *	FSTree_new_scroller.cc
 *	======================
 */

#include "Genie/FileSystem/FSTree_new_scroller.hh"

// Nucleus
#include "Nucleus/Saved.h"

// ClassicToolbox
#include "ClassicToolbox/MacWindows.h"

// Pedestal
#include "Pedestal/EmptyView.hh"
#include "Pedestal/Scroller_beta.hh"

// Genie
#include "Genie/FileSystem/FSTree_Directory.hh"
#include "Genie/FileSystem/FSTree_Property.hh"
#include "Genie/FileSystem/FSTree_sys_window_REF.hh"
#include "Genie/FileSystem/ScrollerBase.hh"
#include "Genie/FileSystem/Views.hh"


namespace Genie
{
	
	namespace N = Nitrogen;
	namespace NN = Nucleus;
	namespace p7 = poseven;
	namespace Ped = Pedestal;
	
	
	struct BasicScrollerParameters
	{
		boost::shared_ptr< Ped::View >  itsSubview;
		
		BasicScrollerParameters() : itsSubview( Ped::EmptyView::Get() )
		{
		}
	};
	
	typedef std::map< const FSTree*, BasicScrollerParameters > BasicScrollerParametersMap;
	
	static BasicScrollerParametersMap  gBasicScrollerParametersMap;
	
	
	class BasicScroller : public ScrollerBase
	{
		private:
			Rect itsSavedBounds;
		
		public:
			BasicScroller( Key key ) : ScrollerBase( key )
			{
			}
			
			View& Subview();
			
			void Draw( const Rect& bounds, bool erasing );
			
			void Scroll( int dh, int dv );
	};
	
	Ped::View& BasicScroller::Subview()
	{
		boost::shared_ptr< Ped::View >& subview = gBasicScrollerParametersMap[ GetKey() ].itsSubview;
		
		if ( subview.get() == NULL )
		{
			subview = Ped::EmptyView::Get();
		}
		
		return *subview;
	}
	
	void BasicScroller::Draw( const Rect& bounds, bool erasing )
	{
		NN::Saved< N::Clip_Value > savedClip;
		
		N::SetClip( N::SectRgn( N::RectRgn( bounds ), N::GetClip() ) );
		
		Rect area = { 0 };
		
		area.right  = ClientWidth();
		area.bottom = ClientHeight();
		
		short dx = bounds.left - GetHOffset();
		short dy = bounds.top  - GetVOffset();
		
		::OffsetRect( &area, dx, dy );
		
		Subview().Draw( area, erasing );
	}
	
	void BasicScroller::Scroll( int dh, int dv )
	{
		Rect bounds = GetScrollerLastViewBounds( GetKey() );
		
		NN::Saved< N::Clip_Value > savedClip;
		
		N::SetClip( N::ScrollRect( bounds, dh, dv ) );
		
		Draw( bounds, true );
		
		N::ValidRect( bounds );
	}
	
	
	static boost::shared_ptr< Ped::View > CreateView( const FSTree* delegate )
	{
		boost::shared_ptr< Ped::View > scroller( new BasicScroller( delegate ) );
		
		return scroller;
	}
	
	
	static void DestroyDelegate( const FSTree* delegate )
	{
		RemoveScrollerParams( delegate );
		
		gBasicScrollerParametersMap.erase( delegate );
		
		RemoveAllViewParameters( delegate );
	}
	
	
	namespace
	{
		
		int& Width( const FSTree* view )
		{
			return GetScrollerParams( view ).itsClientWidth;
		}
		
		int& Height( const FSTree* view )
		{
			return GetScrollerParams( view ).itsClientHeight;
		}
		
		int& HOffset( const FSTree* view )
		{
			return GetScrollerParams( view ).itsHOffset;
		}
		
		int& VOffset( const FSTree* view )
		{
			return GetScrollerParams( view ).itsVOffset;
		}
		
		boost::shared_ptr< Ped::View >& GetView( const FSTree* key )
		{
			return gBasicScrollerParametersMap[ key ].itsSubview;
		}
		
	}
	
	
	template < class Property >
	static FSTreePtr Property_Factory( const FSTreePtr&    parent,
	                                   const std::string&  name )
	{
		return New_FSTree_Property( parent,
		                            name,
		                            &Property::Get,
		                            &Property::Set );
	}
	
	static const FSTree_Premapped::Mapping local_mappings[] =
	{
		{ "width",  &Property_Factory< View_Property< Integer_Scribe< int >, Width  > > },
		{ "height", &Property_Factory< View_Property< Integer_Scribe< int >, Height > > },
		
		{ "x", &Property_Factory< View_Property< Integer_Scribe< int >, HOffset > > },
		{ "y", &Property_Factory< View_Property< Integer_Scribe< int >, VOffset > > },
		
		{ "v", &Basic_Factory< FSTree_X_view< GetView > >, true },
		
		{ NULL, NULL }
	};
	
	FSTreePtr New_FSTree_new_scroller( const FSTreePtr& parent, const std::string& name )
	{
		return FSTreePtr( new FSTree_new_View( parent,
		                                       name,
		                                       &CreateView,
		                                       local_mappings,
		                                       &DestroyDelegate ) );
	}
	
}

