// Nitrogen/Lists.hh
// -----------------
//
// Maintained by Joshua Juran

// Part of the Nitrogen project.
//
// Written 2004 by Joshua Juran.
//
// This code was written entirely by the above contributor, who places it
// in the public domain.


#ifndef NITROGEN_LISTS_HH
#define NITROGEN_LISTS_HH

// Mac OS X
#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

// Mac OS
#ifndef __LISTS__
#include <Lists.h>
#endif

// iota
#include "iota/string_traits.hh"

// nucleus
#ifndef NUCLEUS_OWNED_HH
#include "nucleus/owned.hh"
#endif
#ifndef NUCLEUS_STRING_HH
#include "nucleus/string.hh"
#endif

// Nitrogen
#ifndef MAC_LISTS_TYPES_LISTHANDLE_HH
#include "Mac/Lists/Types/ListHandle.hh"
#endif
#ifndef MAC_RESOURCES_TYPES_RESID_HH
#include "Mac/Resources/Types/ResID.hh"
#endif


namespace Nitrogen
{
	
	void RegisterListManagerErrors();
	
	// 436
	nucleus::owned< ListHandle > LNew( const Rect&        rView,
	                                   const ListBounds&  dataBounds,
	                                   Point              cSize,
	                                   Mac::ResID         theProc,
	                                   WindowRef          theWindow,
	                                   bool               drawIt,
	                                   bool               hasGrow,
	                                   bool               scrollHoriz,
	                                   bool               scrollVert );
	
	// 457
	inline void LDispose( nucleus::owned< ListHandle > )  {}
	
	// 469, 484, 499, 514
	using ::LAddColumn;
	using ::LAddRow;
	using ::LDelColumn;
	using ::LDelRow;
	
	// 529
	inline bool LGetSelect( bool next, Cell& cell, ListHandle lHandle )
	{
		return ::LGetSelect( next, &cell, lHandle );
	}
	
	// 544
	using ::LLastClick;
	
	// LNextCell()
	// LSearch()
	
	// 589
	void LSize( short width, short height, ListHandle list );
	
	// 604
	inline void LSetDrawingMode( bool drawIt, ListHandle lHandle )
	{
		::LSetDrawingMode( drawIt, lHandle );
	}
	
	// 618, 633, 645
	using ::LScroll;
	using ::LAutoScroll;
	using ::LUpdate;
	
	// 659
	inline void LActivate( bool act, ListHandle lHandle )  { ::LActivate( act, lHandle ); }
	
	// 673
	using ::LCellSize;
	
	// 687
	inline bool LClick( Point pt, EventModifiers modifiers, ListHandle lHandle )
	{
		return ::LClick( pt, modifiers, lHandle );
	}
	
	// 702
	using ::LAddToCell;
	
	template < class String >
	inline void LAddToCell( const String& data, Cell cell, ListHandle lHandle )
	{
		using iota::get_string_data;
		using iota::get_string_size;
		
		::LAddToCell( get_string_data( data ),
		              get_string_size( data ),
		              cell,
		              lHandle );
	}
	
	// 718
	using ::LClrCell;
	
	// 732
	nucleus::string LGetCell( Cell cell, ListHandle lHandle );
	
	// 748
	Rect LRect( Cell cell, ListHandle lHandle );
	
	// 763
	using ::LSetCell;
	
	template < class String >
	inline void LSetCell( const String& data, Cell cell, ListHandle lHandle )
	{
		using iota::get_string_data;
		using iota::get_string_size;
		
		::LSetCell( get_string_data( data ),
		            get_string_size( data ),
		            cell,
		            lHandle );
	}
	
	// 779
	inline void LSetSelect( bool setIt, Cell cell, ListHandle lHandle )
	{
		::LSetSelect( setIt, cell, lHandle );
	}
	
	// 794
	using ::LDraw;
	
	struct LGetCellDataLocation_Result
	{
		short offset;
		short len;
	};
	
	// 808
	LGetCellDataLocation_Result LGetCellDataLocation( Cell cell, ListHandle lHandle );
	
	// 1089
	Rect GetListViewBounds( ListHandle list );
	
	// 1143
	ListBounds GetListVisibleCells( ListHandle list );
	
	// 1281
	ListBounds GetListDataBounds( ListHandle list );
	
}

#endif
