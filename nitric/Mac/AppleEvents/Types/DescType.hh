/*
	Mac/AppleEvents/Types/DescType.hh
	---------------------------------
*/

#ifndef MAC_APPLEEVENTS_TYPES_DESCTYPE_HH
#define MAC_APPLEEVENTS_TYPES_DESCTYPE_HH

// Mac OS X
#ifdef __MACH__
#if !defined( __HIOBJECT__ )  ||  !defined( __HITOOLBAR__ )
#include <Carbon/Carbon.h>
#endif
#endif

// Mac OS
#ifndef __AEDATAMODEL__
#include <AEDataModel.h>
#endif
#ifndef __AEOBJECTS__
#include <AEObjects.h>
#endif
#ifndef __AEREGISTRY__
#include <AERegistry.h>
#endif
#ifndef __ASREGISTRY__
#include <ASRegistry.h>
#endif
#ifndef __CARBONEVENTS__
#include <CarbonEvents.h>
#endif

// nucleus
#ifndef NUCLEUS_ENUMERATIONTRAITS_HH
#include "nucleus/enumeration_traits.hh"
#endif


namespace Mac
{
	
	enum DescType
	{
		
		// AEDataModel
		// -----------
		
		typeBoolean                = ::typeBoolean,
		typeChar                   = ::typeChar,
		
		typeSInt16                 = ::typeSInt16,
		typeSInt32                 = ::typeSInt32,
		typeUInt32                 = ::typeUInt32,
		typeSInt64                 = ::typeSInt64,
		typeIEEE32BitFloatingPoint = ::typeIEEE32BitFloatingPoint,
		typeIEEE64BitFloatingPoint = ::typeIEEE64BitFloatingPoint,
		type128BitFloatingPoint    = ::type128BitFloatingPoint,
		typeDecimalStruct          = ::typeDecimalStruct,
		
		typeAEList                 = ::typeAEList,
		typeAERecord               = ::typeAERecord,
		typeAppleEvent             = ::typeAppleEvent,
		typeEventRecord            = ::typeEventRecord,
		typeTrue                   = ::typeTrue,
		typeFalse                  = ::typeFalse,
		typeAlias                  = ::typeAlias,
		typeEnumerated             = ::typeEnumerated,
		typeType                   = ::typeType,
		typeAppParameters          = ::typeAppParameters,
		typeProperty               = ::typeProperty,
		typeFSS                    = ::typeFSS,
		typeFSRef                  = ::typeFSRef,
		typeFileURL                = ::typeFileURL,
		typeKeyword                = ::typeKeyword,
		typeSectionH               = ::typeSectionH,
		typeWildCard               = ::typeWildCard,
		typeApplSignature          = ::typeApplSignature,
		typeQDRectangle            = ::typeQDRectangle,
		typeFixed                  = ::typeFixed,
		typeProcessSerialNumber    = ::typeProcessSerialNumber,
		typeApplicationURL         = ::typeApplicationURL,
		typeNull                   = ::typeNull,
		
	#if CALL_NOT_IN_CARBON
		
		// Deprecated addressing modes under Carbon
		typeSessionID    = ::typeSessionID,
		typeTargetID     = ::typeTargetID,
		typeDispatcherID = ::typeDispatcherID,
		
	#endif
		
		// AEObjects
		// ---------
		
		typeObjectSpecifier     = ::typeObjectSpecifier,
		typeObjectBeingExamined = ::typeObjectBeingExamined,
		typeCurrentContainer    = ::typeCurrentContainer,
		typeToken               = ::typeToken,
		typeRelativeDescriptor  = ::typeRelativeDescriptor,
		typeAbsoluteOrdinal     = ::typeAbsoluteOrdinal,
		typeIndexDescriptor     = ::typeIndexDescriptor,
		typeRangeDescriptor     = ::typeRangeDescriptor,
		typeLogicalDescriptor   = ::typeLogicalDescriptor,
		typeCompDescriptor      = ::typeCompDescriptor,
		typeOSLTokenList        = ::typeOSLTokenList,
		
		// AERegistry
		// ----------
		
		typeAEText             = ::typeAEText,
		typeArc                = ::typeArc,
		typeBest               = ::typeBest,
		typeCell               = ::typeCell,
		typeClassInfo          = ::typeClassInfo,
		typeColorTable         = ::typeColorTable,
		typeColumn             = ::typeColumn,
		typeDashStyle          = ::typeDashStyle,
		typeData               = ::typeData,
		typeDrawingArea        = ::typeDrawingArea,
		typeElemInfo           = ::typeElemInfo,
		typeEnumeration        = ::typeEnumeration,
		typeEPS                = ::typeEPS,
		typeEventInfo          = ::typeEventInfo,
		typeFinderWindow       = ::typeFinderWindow,
		typeFixedPoint         = ::typeFixedPoint,
		typeFixedRectangle     = ::typeFixedRectangle,
		typeGraphicLine        = ::typeGraphicLine,
		typeGraphicText        = ::typeGraphicText,
		typeGroupedGraphic     = ::typeGroupedGraphic,
		typeInsertionLoc       = ::typeInsertionLoc,
		typeIntlText           = ::typeIntlText,
		typeIntlWritingCode    = ::typeIntlWritingCode,
		typeLongDateTime       = ::typeLongDateTime,
		typeLongFixed          = ::typeLongFixed,
		typeLongFixedPoint     = ::typeLongFixedPoint,
		typeLongFixedRectangle = ::typeLongFixedRectangle,
		typeLongPoint          = ::typeLongPoint,
		typeLongRectangle      = ::typeLongRectangle,
		typeMachineLoc         = ::typeMachineLoc,
		typeOval               = ::typeOval,
		typeParamInfo          = ::typeParamInfo,
		typePict               = ::typePict,
		typePixelMap           = ::typePixelMap,
		typePixMapMinus        = ::typePixMapMinus,
		typePolygon            = ::typePolygon,
		typePropInfo           = ::typePropInfo,
		typePtr                = ::typePtr,
		typeQDPoint            = ::typeQDPoint,
		typeQDRegion           = ::typeQDRegion,
		typeRectangle          = ::typeRectangle,
		typeRGB16              = ::typeRGB16,
		typeRGB96              = ::typeRGB96,
		typeRGBColor           = ::typeRGBColor,
		typeRotation           = ::typeRotation,
		typeRoundedRectangle   = ::typeRoundedRectangle,
		typeRow                = ::typeRow,
		typeScrapStyles        = ::typeScrapStyles,
		typeScript             = ::typeScript,
		typeStyledText         = ::typeStyledText,
		typeSuiteInfo          = ::typeSuiteInfo,
		typeTable              = ::typeTable,
		typeTextStyles         = ::typeTextStyles,
		typeTIFF               = ::typeTIFF,
		typeVersion            = ::typeVersion,
		
		// typeHIMenu, typeHIWindow
		
		typeTextRange           = ::typeTextRange,
		typeComponentInstance   = ::typeComponentInstance,
		typeOffsetArray         = ::typeOffsetArray,
		typeTextRangeArray      = ::typeTextRangeArray,
		typeLowLevelEventRecord = ::typeLowLevelEventRecord,
		typeEventRef            = ::typeEventRef,
		typeText                = ::typeText,
		
		typeUnicodeText       = ::typeUnicodeText,
		typeStyledUnicodeText = ::typeStyledUnicodeText,
		typeEncodedString     = ::typeEncodedString,
		typeCString           = ::typeCString,
		typePString           = ::typePString,
		
		// typeMeters .. typeDegreesK
		
		// ASRegistry
		// ----------
		
		typeAETE = ::typeAETE,
		typeAEUT = ::typeAEUT,
		
		typeScszResource = ::typeScszResource,
		
		typeSound = ::typeSound,
		
		// CarbonEvents
		// ------------
		
		typeFSVolumeRefNum = ::typeFSVolumeRefNum,
		
		typeEventTargetRef = ::typeEventTargetRef,
		
		typeWindowRef    = ::typeWindowRef,
		typeGrafPtr      = ::typeGrafPtr,
		typeGWorldPtr    = ::typeGWorldPtr,
		typeDragRef      = ::typeDragRef,
		typeMenuRef      = ::typeMenuRef,
		typeControlRef   = ::typeControlRef,
		typeCollection   = ::typeCollection,
		typeQDRgnHandle  = ::typeQDRgnHandle,
		typeOSStatus     = ::typeOSStatus,
		typeCFStringRef  = ::typeCFStringRef,
		typeCFIndex      = ::typeCFIndex,
		typeCFTypeRef    = ::typeCFTypeRef,
		typeCGContextRef = ::typeCGContextRef,
		typeHIPoint      = ::typeHIPoint,
		typeHISize       = ::typeHISize,
		typeHIRect       = ::typeHIRect,
		
		
		typeMouseButton    = ::typeMouseButton,
		typeMouseWheelAxis = ::typeMouseWheelAxis,
		
		typeEventHotKeyID = ::typeEventHotKeyID,
		
		typeHICommand = ::typeHICommand,
		
		typeWindowRegionCode      = ::typeWindowRegionCode,
		typeWindowDefPartCode     = ::typeWindowDefPartCode,
		typeClickActivationResult = ::typeClickActivationResult,
		
		typeControlActionUPP        = ::typeControlActionUPP,
		typeIndicatorDragConstraint = ::typeIndicatorDragConstraint,
		typeControlPartCode         = ::typeControlPartCode,
		
		typeMenuItemIndex     = ::typeMenuItemIndex,
		typeMenuCommand       = ::typeMenuCommand,
		typeMenuTrackingMode  = ::typeMenuTrackingMode,
		typeMenuEventOptions  = ::typeMenuEventOptions,
		typeThemeMenuState    = ::typeThemeMenuState,
		typeThemeMenuItemType = ::typeThemeMenuItemType,
		
		typeTabletPointRec     = ::typeTabletPointRec,
		typeTabletProximityRec = ::typeTabletProximityRec,
		typeTabletPointerRec   = ::typeTabletPointerRec,
		
		typeScrapRef          = ::typeScrapRef,
		typeCFMutableArrayRef = ::typeCFMutableArrayRef,
		
	#ifdef __MACH__
		
		// HIObject
		// --------
		
		typeHIObjectRef = ::typeHIObjectRef,
		
		// HIToolbar
		// --------
		
		typeHIToolbarRef     = ::typeHIToolbarRef,
		typeHIToolbarItemRef = ::typeHIToolbarItemRef,
		
	#endif
		
		// OSA
		// ---
		
		typeOSAErrorRange     = ::typeOSAErrorRange,
		typeOSAGenericStorage = ::typeOSAGenericStorage,
		
		kDescType_Max = nucleus::enumeration_traits< ::DescType >::max
	};
	
}

#endif

