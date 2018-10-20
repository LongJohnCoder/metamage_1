// Nitrogen/Navigation.hh
// ----------------------
//
// Maintained by Marshall Clow

// Part of the Nitrogen project.
//
// Written 2004-2007 by Marshall Clow and Joshua Juran.
//
// This code was written entirely by the above contributors, who place it
// in the public domain.


#ifndef NITROGEN_NAVIGATION_HH
#define NITROGEN_NAVIGATION_HH

// Mac OS X
#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

// Mac OS
#if ! __LP64__
#ifndef __NAVIGATION__
#include <Navigation.h>
#endif
#endif

// nucleus
#ifndef NUCLEUS_OWNED_HH
#include "nucleus/owned.hh"
#endif

// Nitrogen
#ifndef MAC_FILES_TYPES_FSCREATOR_HH
#include "Mac/Files/Types/FSCreator.hh"
#endif
#ifndef MAC_FILES_TYPES_FSTYPE_HH
#include "Mac/Files/Types/FSType.hh"
#endif
#ifndef MAC_TOOLBOX_UTILITIES_THROWOSSTATUS_HH
#include "Mac/Toolbox/Utilities/ThrowOSStatus.hh"
#endif


#if 0
typedef UInt32 NavAskSaveChangesAction;
enum {
                                        /* input action codes for NavAskSaveChanges() */
  kNavSaveChangesClosingDocument = 1,
  kNavSaveChangesQuittingApplication = 2,
  kNavSaveChangesOther          = 0
};


typedef UInt32 NavAskSaveChangesResult;
enum {
                                        /* result codes for NavAskSaveChanges() */
  kNavAskSaveChangesSave        = 1,
  kNavAskSaveChangesCancel      = 2,
  kNavAskSaveChangesDontSave    = 3
};


typedef UInt32 NavAskDiscardChangesResult;
enum {
                                        /* result codes for NavAskDiscardChanges() */
  kNavAskDiscardChanges         = 1,
  kNavAskDiscardChangesCancel   = 2
};


typedef SInt16 NavFilterModes;
enum {
                                        /* which elements are being filtered for objects: */
  kNavFilteringBrowserList      = 0,
  kNavFilteringFavorites        = 1,
  kNavFilteringRecents          = 2,
  kNavFilteringShortCutVolumes  = 3,
  kNavFilteringLocationPopup    = 4     /* for v1.1 or greater */
};


enum {
  kNavFileOrFolderVersion       = 1
};
#endif

namespace Nitrogen {

	class NavServicesErrorsRegistrationDependency
	{
		public:
			NavServicesErrorsRegistrationDependency();
	};
	
  }

#if ! __LP64__
	
namespace nucleus
  {
	template <> struct disposer< NavDialogRef >
		{
		typedef NavDialogRef  argument_type;
		typedef void          result_type;
		
		void operator()( NavDialogRef nav ) const
			{
				::NavDialogDispose ( nav );
			}
		};
/*
	template <> struct disposer< NavReplyRecord >
		{
		typedef NavReplyRecord  argument_type;
		typedef void            result_type;
		
		void operator()( NavReplyRecord &nav ) const
			{
				::NavDisposeReply ( &nav );
			}
		};
*/
  }

namespace Nitrogen
  {

#if 0
typedef UInt32 NavUserAction;

typedef SInt32 NavEventCallbackMessage;
#endif

#if 0
/*
 *  NewNavEventUPP()
 *  
 *  Availability:
 *    Mac OS X:         in version 10.0 and later in Carbon.framework
 *    CarbonLib:        in CarbonLib 1.0 and later
 *    Non-Carbon CFM:   available as macro/inline
 */
extern NavEventUPP
NewNavEventUPP(NavEventProcPtr userRoutine)                   AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;

/*
 *  NewNavPreviewUPP()
 *  
 *  Availability:
 *    Mac OS X:         in version 10.0 and later in Carbon.framework
 *    CarbonLib:        in CarbonLib 1.0 and later
 *    Non-Carbon CFM:   available as macro/inline
 */
extern NavPreviewUPP
NewNavPreviewUPP(NavPreviewProcPtr userRoutine)               AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;

/*
 *  NewNavObjectFilterUPP()
 *  
 *  Availability:
 *    Mac OS X:         in version 10.0 and later in Carbon.framework
 *    CarbonLib:        in CarbonLib 1.0 and later
 *    Non-Carbon CFM:   available as macro/inline
 */
extern NavObjectFilterUPP
NewNavObjectFilterUPP(NavObjectFilterProcPtr userRoutine)     AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;

/*
 *  DisposeNavEventUPP()
 *  
 *  Availability:
 *    Mac OS X:         in version 10.0 and later in Carbon.framework
 *    CarbonLib:        in CarbonLib 1.0 and later
 *    Non-Carbon CFM:   available as macro/inline
 */
extern void
DisposeNavEventUPP(NavEventUPP userUPP)                       AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;

/*
 *  DisposeNavPreviewUPP()
 *  
 *  Availability:
 *    Mac OS X:         in version 10.0 and later in Carbon.framework
 *    CarbonLib:        in CarbonLib 1.0 and later
 *    Non-Carbon CFM:   available as macro/inline
 */
extern void
DisposeNavPreviewUPP(NavPreviewUPP userUPP)                   AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;

/*
 *  DisposeNavObjectFilterUPP()
 *  
 *  Availability:
 *    Mac OS X:         in version 10.0 and later in Carbon.framework
 *    CarbonLib:        in CarbonLib 1.0 and later
 *    Non-Carbon CFM:   available as macro/inline
 */
extern void
DisposeNavObjectFilterUPP(NavObjectFilterUPP userUPP)         AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;

/*
 *  InvokeNavEventUPP()
 *  
 *  Availability:
 *    Mac OS X:         in version 10.0 and later in Carbon.framework
 *    CarbonLib:        in CarbonLib 1.0 and later
 *    Non-Carbon CFM:   available as macro/inline
 */
extern void
InvokeNavEventUPP(
  NavEventCallbackMessage  callBackSelector,
  NavCBRecPtr              callBackParms,
  void *                   callBackUD,
  NavEventUPP              userUPP)                           AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;

/*
 *  InvokeNavPreviewUPP()
 *  
 *  Availability:
 *    Mac OS X:         in version 10.0 and later in Carbon.framework
 *    CarbonLib:        in CarbonLib 1.0 and later
 *    Non-Carbon CFM:   available as macro/inline
 */
extern Boolean
InvokeNavPreviewUPP(
  NavCBRecPtr    callBackParms,
  void *         callBackUD,
  NavPreviewUPP  userUPP)                                     AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;

/*
 *  InvokeNavObjectFilterUPP()
 *  
 *  Availability:
 *    Mac OS X:         in version 10.0 and later in Carbon.framework
 *    CarbonLib:        in CarbonLib 1.0 and later
 *    Non-Carbon CFM:   available as macro/inline
 */
extern Boolean
InvokeNavObjectFilterUPP(
  AEDesc *            theItem,
  void *              info,
  void *              callBackUD,
  NavFilterModes      filterMode,
  NavObjectFilterUPP  userUPP)                                AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;
#endif

#if 0
typedef SInt32 NavCustomControlMessage;

typedef UInt32 NavActionState;

typedef UInt16 NavPopupMenuItem;

typedef UInt16 NavSortKeyField;

typedef UInt16 NavSortOrder;

typedef UInt32 NavDialogOptionFlags;

typedef UInt32 NavTranslationOptions;
#endif


//	extern UInt32 NavLibraryVersion(void)

/*	We don't provide Nitrogen versions of the deprecated functions:
		NavChooseFile
		NavChooseFolder
		NavChooseObject
		NavChooseVolume
		NavGetFile
		NavNewFolder
		NavAskDiscardChanges
		NavAskSaveChanges
		NavCustomAskSaveChanges
		NavPutFile
		NavGetDefaultDialogOptions
		
or the unsupported functions:
		NavLoad
		NavUnload
		NavServicesCanRun
*/


	

#if 0
/*
 *  NavTranslateFile()
 *  
 *  Availability:
 *    Mac OS X:         in version 10.0 and later in Carbon.framework
 *    CarbonLib:        in CarbonLib 1.0 and later
 *    Non-Carbon CFM:   in NavigationLib 1.0 and later
 */
extern OSErr 
NavTranslateFile(
  NavReplyRecord *        reply,
  NavTranslationOptions   howToTranslate)                     AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;
#endif


//	Semantics?? What should happen to the NavReplyRecord here?
	inline void NavCompleteSave ( NavReplyRecord &reply, NavTranslationOptions howToTranslate ) {
		(void) NavServicesErrorsRegistrationDependency();
		Mac::ThrowOSStatus ( ::NavCompleteSave ( &reply, howToTranslate ));
		}

	inline void NavCustomControl ( NavDialogRef dialog, NavCustomControlMessage selector, void *parms ) {
		(void) NavServicesErrorsRegistrationDependency();
		Mac::ThrowOSStatus ( ::NavCustomControl ( dialog, selector, parms ));
		}

	inline void NavCreatePreview (
			AEDesc *theObject,
			OSType        previewDataType	= OSType(),
			const void *  previewData		= NULL,
			std::size_t   previewDataSize	= 0 ) {
		(void) NavServicesErrorsRegistrationDependency();
		Mac::ThrowOSStatus ( ::NavCreatePreview ( theObject, previewDataType, previewData, previewDataSize ));
		}
	
	inline void NavDisposeReply ( NavReplyRecord * reply ) {
		(void) NavServicesErrorsRegistrationDependency();
		Mac::ThrowOSStatus ( ::NavDisposeReply ( reply ));
		}		
	
	using ::NavServicesAvailable;

	inline NavDialogCreationOptions NavGetDefaultDialogCreationOptions(void) {
		(void) NavServicesErrorsRegistrationDependency();
		NavDialogCreationOptions result;
		Mac::ThrowOSStatus ( ::NavGetDefaultDialogCreationOptions ( &result ));
		return result;
		}
	
	inline nucleus::owned<NavDialogRef> NavCreateGetFileDialog (
			const NavDialogCreationOptions *  inOptions 	= NULL,
			NavTypeListHandle                 inTypeList	= NULL,
			NavEventUPP                       inEventProc	= NULL,
			NavPreviewUPP                     inPreviewProc	= NULL,
			NavObjectFilterUPP                inFilterProc	= NULL,
			void *                            inClientData 	= NULL ) {
		(void) NavServicesErrorsRegistrationDependency();
		NavDialogRef result;
		Mac::ThrowOSStatus ( ::NavCreateGetFileDialog ( inOptions, inTypeList,
					inEventProc, inPreviewProc, inFilterProc, inClientData, &result ));
		return nucleus::owned<NavDialogRef>::seize ( result );
		}

	inline nucleus::owned<NavDialogRef> NavCreatePutFileDialog (
			const NavDialogCreationOptions *  inOptions, /* Can be NULL */
			Mac::FSType                       inFileType,
			Mac::FSCreator                    inFileCreator,
			NavEventUPP                       inEventProc	= NULL,
			void *                            inClientData 	= NULL ) {
		(void) NavServicesErrorsRegistrationDependency();
		NavDialogRef result;
		Mac::ThrowOSStatus ( ::NavCreatePutFileDialog ( inOptions, inFileType, inFileCreator,
					inEventProc, inClientData, &result ));
		return nucleus::owned<NavDialogRef>::seize ( result );
		}

	inline nucleus::owned<NavDialogRef> NavCreateAskReviewDocumentsDialog (
			const NavDialogCreationOptions *  inOptions,
			UInt32                            inDocumentCount,
			NavEventUPP                       inEventProc	= NULL,
			void *                            inClientData 	= NULL ) {
		(void) NavServicesErrorsRegistrationDependency();
		NavDialogRef result;
		Mac::ThrowOSStatus ( ::NavCreateAskReviewDocumentsDialog ( inOptions, inDocumentCount,
					inEventProc, inClientData, &result ));
		return nucleus::owned<NavDialogRef>::seize ( result );
		}

	inline nucleus::owned<NavDialogRef> NavCreateAskSaveChangesDialog (
			const NavDialogCreationOptions *  inOptions,
			NavAskSaveChangesAction           inAction,
			NavEventUPP                       inEventProc	= NULL,
			void *                            inClientData 	= NULL ) {
		(void) NavServicesErrorsRegistrationDependency();
		NavDialogRef result;
		Mac::ThrowOSStatus ( ::NavCreateAskSaveChangesDialog ( inOptions, inAction,
					inEventProc, inClientData, &result ));
		return nucleus::owned<NavDialogRef>::seize ( result );
		}

	inline nucleus::owned<NavDialogRef> NavCreateAskDiscardChangesDialog (
			const NavDialogCreationOptions *  inOptions, /* Can be NULL */
			NavEventUPP                       inEventProc	= NULL,
			void *                            inClientData 	= NULL ) {
		(void) NavServicesErrorsRegistrationDependency();
		NavDialogRef result;
		Mac::ThrowOSStatus ( ::NavCreateAskDiscardChangesDialog ( inOptions,
					inEventProc, inClientData, &result ));
		return nucleus::owned<NavDialogRef>::seize ( result );
		}

	inline nucleus::owned<NavDialogRef> NavCreateChooseFileDialog (
			const NavDialogCreationOptions *  inOptions 	= NULL,
			NavTypeListHandle                 inTypeList	= NULL,
			NavEventUPP                       inEventProc	= NULL,
			NavPreviewUPP                     inPreviewProc	= NULL,
			NavObjectFilterUPP                inFilterProc	= NULL,
			void *                            inClientData 	= NULL ) {
		(void) NavServicesErrorsRegistrationDependency();
		NavDialogRef result;
		Mac::ThrowOSStatus ( ::NavCreateChooseFileDialog ( inOptions, inTypeList,
					inEventProc, inPreviewProc, inFilterProc, inClientData, &result ));
		return nucleus::owned<NavDialogRef>::seize ( result );
		}

	inline nucleus::owned<NavDialogRef> NavCreateChooseFolderDialog (
			const NavDialogCreationOptions *  inOptions 	= NULL,
			NavEventUPP                       inEventProc	= NULL,
			NavObjectFilterUPP                inFilterProc	= NULL,
			void *                            inClientData 	= NULL ) {
		(void) NavServicesErrorsRegistrationDependency();
		NavDialogRef result;
		Mac::ThrowOSStatus ( ::NavCreateChooseFolderDialog ( inOptions,
					inEventProc, inFilterProc, inClientData, &result ));
		return nucleus::owned<NavDialogRef>::seize ( result );
		}

	inline nucleus::owned<NavDialogRef> NavCreateChooseVolumeDialog (
			const NavDialogCreationOptions *  inOptions 	= NULL,
			NavEventUPP                       inEventProc	= NULL,
			NavObjectFilterUPP                inFilterProc	= NULL,
			void *                            inClientData 	= NULL ) {
		(void) NavServicesErrorsRegistrationDependency();
		NavDialogRef result;
		Mac::ThrowOSStatus ( ::NavCreateChooseVolumeDialog ( inOptions,
					inEventProc, inFilterProc, inClientData, &result ));
		return nucleus::owned<NavDialogRef>::seize ( result );
		}

	inline nucleus::owned<NavDialogRef> NavCreateChooseObjectDialog (
			const NavDialogCreationOptions *  inOptions 	= NULL,
			NavEventUPP                       inEventProc	= NULL,
			NavPreviewUPP                     inPreviewProc	= NULL,
			NavObjectFilterUPP                inFilterProc	= NULL,
			void *                            inClientData 	= NULL ) {
		(void) NavServicesErrorsRegistrationDependency();
		NavDialogRef result;
		Mac::ThrowOSStatus ( ::NavCreateChooseObjectDialog ( inOptions,
					inEventProc, inPreviewProc, inFilterProc, inClientData, &result ));
		return nucleus::owned<NavDialogRef>::seize ( result );
		}

	inline nucleus::owned<NavDialogRef> NavCreateNewFolderDialog (
			const NavDialogCreationOptions *  inOptions 	= NULL,
			NavEventUPP                       inEventProc	= NULL,
			void *                            inClientData 	= NULL ) {
		(void) NavServicesErrorsRegistrationDependency();
		NavDialogRef result;
		Mac::ThrowOSStatus ( ::NavCreateNewFolderDialog ( inOptions,
					inEventProc, inClientData, &result ));
		return nucleus::owned<NavDialogRef>::seize ( result );
		}

	inline void NavDialogRun ( NavDialogRef inDialog ) {
		(void) NavServicesErrorsRegistrationDependency();
		Mac::ThrowOSStatus ( ::NavDialogRun ( inDialog ));
		}

//	Since we don't want people calling NavDialogDispose, we're not 
//	providing a Nitrogen version

//	extern WindowRef NavDialogGetWindow(NavDialogRef inDialog);
	using ::NavDialogGetWindow;

//	extern NavUserAction NavDialogGetUserAction(NavDialogRef inDialog);
	using ::NavDialogGetUserAction;

#if 0
	inline nucleus::owned<NavReplyRecord> NavDialogGetReply ( NavDialogRef inDialog ) {
		(void) NavServicesErrorsRegistrationDependency();
		NavReplyRecord result;
		Mac::ThrowOSStatus ( ::NavDialogGetReply ( inDialog, &result ));
		return nucleus::owned<NavReplyRecord>::seize ( result );
		}
#endif
	inline NavReplyRecord NavDialogGetReply ( NavDialogRef inDialog ) {
		(void) NavServicesErrorsRegistrationDependency();
		NavReplyRecord result;
		Mac::ThrowOSStatus ( ::NavDialogGetReply ( inDialog, &result ));
		return result;
		}

//	extern CFStringRef NavDialogGetSaveFileName(NavDialogRef inPutFileDialog);
	using ::NavDialogGetSaveFileName;

	inline void NavDialogSetSaveFileName ( NavDialogRef inPutFileDialog, CFStringRef inFileName ) {
		(void) NavServicesErrorsRegistrationDependency();
		Mac::ThrowOSStatus ( ::NavDialogSetSaveFileName ( inPutFileDialog, inFileName ));
		}

	inline bool NavDialogGetSaveFileExtensionHidden ( NavDialogRef inPutFileDialog ) {
		return ::NavDialogGetSaveFileExtensionHidden ( inPutFileDialog );
		}

	inline void NavDialogSetSaveFileExtensionHidden ( NavDialogRef inPutFileDialog, bool inHidden ) {
		(void) NavServicesErrorsRegistrationDependency();
		Mac::ThrowOSStatus ( ::NavDialogSetSaveFileExtensionHidden ( inPutFileDialog, inHidden ));
		}

	}

#endif  // #if ! __LP64__

#endif /* NITROGEN_NAVIGATION_HH */
