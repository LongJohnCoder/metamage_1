// Nitrogen/Controls.hh
// --------------------
//
// Maintained by Joshua Juran

// Part of the Nitrogen project.
//
// Written 2002-2009 by Lisa Lippincott, Marshall Clow, and Joshua Juran.
//
// This code was written entirely by the above contributors, who place it
// in the public domain.


#ifndef NITROGEN_CONTROLS_HH
#define NITROGEN_CONTROLS_HH

// Mac OS X
#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

// Mac OS
#ifndef __CONTROLS__
#include <Controls.h>
#endif

// iota
#include "iota/ptr_diff.hh"
#include "iota/string_traits.hh"

// nucleus
#ifndef NUCLEUS_ENUMERATIONTRAITS_HH
#include "nucleus/enumeration_traits.hh"
#endif
#ifndef NUCLEUS_ERRORSREGISTERED_HH
#include "nucleus/errors_registered.hh"
#endif
#ifndef NUCLEUS_MAKE_HH
#include "nucleus/make.hh"
#endif
#ifndef NUCLEUS_SCRIBE_HH
#include "nucleus/scribe.hh"
#endif

// Nitrogen
#ifndef MAC_CONTROLS_TYPES_CONTROLDATATAG_HH
#include "Mac/Controls/Types/ControlData_Tag.hh"
#endif
#ifndef MAC_CONTROLS_TYPES_CONTROLPARTCODE_HH
#include "Mac/Controls/Types/ControlPartCode.hh"
#endif
#ifndef MAC_CONTROLS_TYPES_CONTROLPROCID_HH
#include "Mac/Controls/Types/ControlProcID.hh"
#endif
#ifndef MAC_TOOLBOX_TYPES_OSTYPE_HH
#include "Mac/Toolbox/Types/OSType.hh"
#endif

#ifndef NITROGEN_REFCON_HH
#include "Nitrogen/RefCon.hh"
#endif
#ifndef NITROGEN_STR_HH
#include "Nitrogen/Str.hh"
#endif
#ifndef NITROGEN_UPP_HH
#include "Nitrogen/UPP.hh"
#endif


#ifdef DrawOneControl
   #undef DrawOneControl
   inline void DrawOneControl( ControlRef theControl )
     {
      Draw1Control( theControl );
     }
#endif

namespace Nitrogen
{
	
	NUCLEUS_DECLARE_ERRORS_DEPENDENCY( ControlManager );
	
  }

#if ! __LP64__

namespace nucleus
  {
   // Not actually used, since controls are owned by the parent window
   
   template <> struct disposer< ControlRef >
     {
      typedef ControlRef  argument_type;
      typedef void        result_type;
      
      void operator()( ControlRef c ) const
        {
         ::DisposeControl( c );
        }
     };
  }

#endif  // #if ! __LP64__

namespace Nitrogen
  {
	
	using Mac::ControlPartCode;
	using Mac::kControlNoPart;
	using Mac::kControlIndicatorPart;
	using Mac::kControlDisabledPart;
	using Mac::kControlInactivePart;
	using Mac::kControlEntireControl;
	
	struct ControlActionUPP_Details : Basic_UPP_Details< ::ControlActionUPP,
	                                                     ::ControlActionProcPtr,
	                                                     ::NewControlActionUPP,
	                                                     ::DisposeControlActionUPP,
	                                                     ::InvokeControlActionUPP >
	{};
	
	typedef UPP< ControlActionUPP_Details > ControlActionUPP;
	
	typedef void ( *ControlActionProcPtr )( ControlRef control, ControlPartCode partCode );
	
	template < ControlActionProcPtr actionProc >
	struct ControlAction_Callback
	{
		static pascal void Adapter( ::ControlRef control, ::ControlPartCode partCode )
		{
			try
			{
				actionProc( control, ControlPartCode( partCode ) );
			}
			catch ( ... )
			{
			}
		}
	};
	
	inline nucleus::owned< ControlActionUPP > NewControlActionUPP( ::ControlActionProcPtr p )
	{
		return NewUPP< ControlActionUPP >( p );
	}

	inline void DisposeControlActionUPP( nucleus::owned< ControlActionUPP > )
	{
	}
	
	inline void InvokeControlActionUPP( ControlRef        theControl, 
	                                    ControlPartCode   partCode,
	                                    ControlActionUPP  userUPP )
	{
		userUPP( theControl, partCode );
	}
	
	using Mac::ControlProcID;
	
	// 972
	ControlRef NewControl( WindowRef         owningWindow,
	                       const Rect&       boundsRect,
	                       ConstStr255Param  controlTitle,
	                       bool              initiallyVisible,
	                       short             initialValue,
	                       short             minimumValue,
	                       short             maximumValue,
	                       ControlProcID     procID,
	                       long              refCon );
	
	inline ControlRef NewControl( WindowRef         owningWindow,
	                              const Rect&       boundsRect,
	                              ConstStr255Param  controlTitle,
	                              bool              initiallyVisible,
	                              short             initialValue,
	                              short             minimumValue,
	                              short             maximumValue,
	                              ControlProcID     procID,
	                              const void*       refCon )
	{
		return NewControl( owningWindow,
		                   boundsRect,
		                   controlTitle,
		                   initiallyVisible,
		                   initialValue,
		                   minimumValue,
		                   maximumValue,
		                   procID,
		                   (long) refCon );  // reinterpret_cast
	}
	
#if ! __LP64__
	
	// 1007
	using ::DisposeControl;
	inline void DisposeControl( nucleus::owned< ControlRef > )  {}
	
	// 1169
	inline void HiliteControl( ControlRef control, ControlPartCode hiliteState )
	{
		::HiliteControl( control, hiliteState );
	}
	
	// 1183, 1195
	using ::ShowControl;
	using ::HideControl;
	
	// 1336
   using ::DrawOneControl;

	// 1339
	inline void UpdateControls( WindowRef window, RgnHandle region )
	{
		::UpdateControls( window, region );
	}
	
	void UpdateControls( WindowRef window );
	
	// 1711
	inline ControlPartCode TrackControl( ControlRef        theControl,
	                                     Point             startPoint,
	                                     ControlActionUPP  actionProc = NULL )
	{
		return ControlPartCode( ::TrackControl( theControl,
		                                        startPoint,
		                                        actionProc ) );
	}
	
	template < typename ControlActionUPP::ProcPtr actionProc >
	inline ControlPartCode TrackControl( ControlRef theControl, Point startPoint )
	{
		return TrackControl( theControl,
		                     startPoint,
		                     StaticUPP< ControlActionUPP, actionProc >() );
	}
	
	template < ControlActionProcPtr actionProc >
	inline ControlPartCode TrackControl( ControlRef theControl, Point startPoint )
	{
		return TrackControl< ControlAction_Callback< actionProc >::Adapter >( theControl,
		                                                                      startPoint );
	}
	
	struct FindControl_Result
	{
		ControlRef       control;
		ControlPartCode  part;
	};
	
	// 1757
	FindControl_Result FindControl( Point testPoint, WindowRef theWindow );
	
	// 1914, 1929
	using ::MoveControl;
	using ::SizeControl;
	
	// 1947
	using ::SetControlTitle;
	
	template < unsigned char n >
	inline void SetControlTitle( ControlRef control, const Str< n >& title )
	{
		::SetControlTitle( control, title + 0 );
	}
	
	template < class String >
	inline void SetControlTitle( ControlRef control, const String& title )
	{
		using iota::get_string_data;
		using iota::get_string_size;
		
		::SetControlTitle( control,
		                   Str255( get_string_data( title ),
		                           get_string_size( title ) ) );
	}
	
	// 2006, 2018, 2032, 2044, 2058, 2070
	using ::GetControlValue;
	using ::SetControlValue;
	using ::GetControlMinimum;
	using ::SetControlMinimum;
	using ::GetControlMaximum;
	using ::SetControlMaximum;
	
#endif  // #if ! __LP64__
	
	// 2211
   using ::ControlID;
   
  }

namespace nucleus
  {
   template <>
   struct maker< ControlID >
     {
      ControlID operator()( Mac::OSType signature, SInt32 id ) const
        {
         ControlID result;
         result.signature = signature;
         result.id = id;
         return result;
        }
     };   
  }

namespace Nitrogen
{
	
	typedef Mac::ControlData_Tag ControlDataTag;
	
   template < ::ResType inTagName > struct ControlData_Traits;

   template<> struct ControlData_Traits< kControlFontStyleTag >: nucleus::POD_scribe< ControlFontStyleRec > {};
   template<> struct ControlData_Traits< kControlKeyFilterTag >: nucleus::POD_scribe< ControlKeyFilterUPP > {};
   template<> struct ControlData_Traits< kControlKindTag      >: nucleus::POD_scribe< ControlKind         > {};
   template<> struct ControlData_Traits< kControlSizeTag      >: nucleus::POD_scribe< ControlSize         > {};

   struct ControlKeyFilterUPP_Details: Basic_UPP_Details< ::ControlKeyFilterUPP,
                                                          ::ControlKeyFilterProcPtr,
                                                          ::NewControlKeyFilterUPP,
                                                          ::DisposeControlKeyFilterUPP,
                                                          ::InvokeControlKeyFilterUPP >
     {};
   
   typedef UPP< ControlKeyFilterUPP_Details > ControlKeyFilterUPP;
   
   // 2245
   ControlRef GetControlByID( WindowRef inWindow, const ControlID& id );
   ControlRef GetControlByID( WindowRef inWindow, Mac::OSType signature, SInt32 id );  // To be removed; use Make.

#if ! __LP64__
	
	// 2491
	inline void SetControlAction( ControlRef control, ControlActionUPP actionProc )
	{
		::SetControlAction( control, actionProc );
	}
	
	template < typename ControlActionUPP::ProcPtr actionProc >
	inline void SetControlAction( ControlRef control )
	{
		SetControlAction( control, StaticUPP< ControlActionUPP, actionProc >() );
	}
	
	template < ControlActionProcPtr actionProc >
	inline void SetControlAction( ControlRef control )
	{
		SetControlAction< ControlAction_Callback< actionProc >::Adapter >( control );
	}
	
	// 2505
	inline ControlActionUPP GetControlAction( ControlRef control )  { return ::GetControlAction( control ); }
	
	// 2520
	using ::SetControlReference;
	
	inline void SetControlReference( ControlRef control, const void* data )
	{
		::SetControlReference( control, (long) data );  // reinterpret_cast
	}
	
	// 2534
	inline RefCon GetControlReference( ControlRef control )
	{
		return ::GetControlReference( control );
	}
	
#endif  // #if ! __LP64__
	

   template < ::ResType inTagName >
   struct SetControlData_Traits: public ControlData_Traits<inTagName>
     {
      typedef typename ControlData_Traits<inTagName>::argument_type InData_Type;
     };

   template < ::ResType inTagName >
   struct GetControlData_Traits: public ControlData_Traits<inTagName>
     {
     };
   
   // 2835
   void SetControlData( ControlRef        inControl,
                        ControlPartCode   inPart,
                        ControlDataTag    inTagName,
                        std::size_t       inSize,
                        const void *      inData );

	class SetControlData_Putter
	{
		private:
			ControlRef       itsControl;
			ControlPartCode  itsPart;
			ControlDataTag   itsTagName;
		
		public:
			SetControlData_Putter( ControlRef       control,
			                       ControlPartCode  part,
			                       ControlDataTag   tagName ) : itsControl( control ),
			                                                    itsPart   ( part    ),
			                                                    itsTagName( tagName )  {}
			
			void operator()( const void *begin, const void *end ) const
			{
				Nitrogen::SetControlData( itsControl,
				                          itsPart,
				                          itsTagName,
				                          iota::ptr_diff( begin, end ),
				                          begin );
			}
	};
	
   template < ::ResType inTagName >
   inline void SetControlData( ControlRef                                               inControl,
                               ControlPartCode                                          inPart,
                               typename SetControlData_Traits< inTagName >::InData_Type inData )
     {
      typedef SetControlData_Traits< inTagName > Traits;
      
      Traits::Put( inData,
		           SetControlData_Putter( inControl,
		                                  inPart,
		                                  ControlDataTag( inTagName ) ) );
     }

   template < ::ResType inTagName >
   inline void SetControlData( ControlRef                                               inControl,
                               typename SetControlData_Traits< inTagName >::InData_Type inData )
     {
      return SetControlData< inTagName >( inControl, kControlEntireControl, inData );
     }

	// 2852
	std::size_t GetControlData( ControlRef       inControl,
	                            ControlPartCode  inPart,
	                            ControlDataTag   inTagName,
	                            std::size_t      inBufferSize,
	                            void *           inBuffer );
	
	inline std::size_t GetControlData( ControlRef       control,
	                                   ControlPartCode  part,
	                                   ControlDataTag   tagName )
	{
		return GetControlData( control, part, tagName, 0, 0 );
	}
	
	template < ::ResType tagName >
	class GetControlData_Getter
	{
		private:
			ControlRef       itsControl;
			ControlPartCode  itsPart;
		
		public:
			GetControlData_Getter( ControlRef       control,
			                       ControlPartCode  part ) : itsControl( control ),
			                                                 itsPart   ( part    )  {}
			
			std::size_t size() const
			{
				typedef GetControlData_Traits< tagName > scribe;
				
				if ( nucleus::scribe_has_static_size< scribe >::value )
				{
					return scribe::static_size;
				}
				
				return GetControlData( itsControl, itsPart, tagName );
			}
			
			void operator()( void *begin, void *end ) const
			{
				GetControlData( itsControl,
				                itsPart,
				                Nitrogen::ControlDataTag( tagName ),
				                iota::ptr_diff( begin, end ),
				                begin );
			}
	};
	
   template < ::ResType inTagName >
   inline typename GetControlData_Traits<inTagName>::result_type
   GetControlData( ControlRef        inControl,
                   ControlPartCode   inPart = kControlEntireControl )
     {
      typedef GetControlData_Traits< inTagName > Traits;
      
      return Traits::Get( GetControlData_Getter< inTagName >( inControl, inPart ) );
     }
   
   /* ... */
   
   // 2918
   bool HandleControlDragTracking( ControlRef          inControl,
                                   DragTrackingMessage inMessage,
                                   DragReference       inDrag );

   void HandleControlDragReceive( ControlRef    inControl,
                                  DragReference inDrag );

   void SetControlDragTrackingEnabled( ControlRef theControl,
                                       bool       tracks );

   bool IsControlDragTrackingEnabled( ControlRef theControl );

   void SetAutomaticControlDragTrackingEnabledForWindow( WindowRef theWindow,
                                                         bool      tracks );

	// 3140
   bool IsAutomaticControlDragTrackingEnabledForWindow( WindowRef theWindow );

#if ! __LP64__
	
	// 3395
	inline Rect GetControlBounds( ControlRef control )
	{
		Rect bounds;
		
		return *( ::GetControlBounds( control, &bounds ) );
	}
	
	// 3497
	inline void SetControlBounds( ControlRef control, const Rect& bounds )
	{
		::SetControlBounds( control, &bounds );
	}
	
#endif  // #if ! __LP64__
	
   /* ... */
   
  }

#endif
