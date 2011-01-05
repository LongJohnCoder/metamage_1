// Nitrogen/Icons.cc
// -----------------
//
// Maintained by Joshua Juran

// Part of the Nitrogen project.
//
// Written 2004-2009 by Lisa Lippincott and Joshua Juran.
//
// This code was written entirely by the above contributors, who place it
// in the public domain.


#include "Nitrogen/Icons.hh"

// Nitrogen
#include "Nitrogen/Folders.hh"
#include "Nitrogen/OSStatus.hh"


namespace Nitrogen
{
	
	using Mac::ThrowOSStatus;
	
	
	// does nothing, but guarantees construction of theRegistration
	NUCLEUS_DEFINE_ERRORS_DEPENDENCY( IconManager )
	
	
	static void RegisterIconManagerErrors();
	
	
#if NUCLEUS_RICH_ERRORCODES
#pragma force_active on
	
	class IconManagerErrorsRegistration
	{
		public:
			IconManagerErrorsRegistration()  { RegisterIconManagerErrors(); }
	};
	
	static IconManagerErrorsRegistration theRegistration;
	
#pragma force_active reset
#endif
	
	
	nucleus::owned< CIconHandle > GetCIcon( ResID iconID )
	{
		CIconHandle h = ::GetCIcon( iconID );
		
		if ( h == NULL )
		{
			MemError();
			ResError();
			
			ThrowOSStatus( resNotFound );
		}
		
		return nucleus::owned< CIconHandle >::seize( h );
	}
	
	void PlotCIcon( const Rect& rect, CIconHandle icon )
	{
		::PlotCIcon( &rect, icon );
	}
	
	PlainIconHandle GetIcon( ResID iconID )
	{
		// Returns a resource handle
		return Handle_Cast< PlainIcon >( Handle( ::GetIcon( iconID ) ) );
	}
	
	void PlotIcon( const Rect& rect, PlainIconHandle icon )
	{
		::PlotIcon( &rect, Handle( icon ) );
	}
	
	void PlotIconID( const Rect&        rect,
	                 IconAlignmentType  align,
	                 IconTransformType  transform,
	                 ResID              resID )
	{
		ThrowOSStatus( ::PlotIconID( &rect, align, transform, resID ) );
	}
	
	nucleus::owned< IconSuiteRef > NewIconSuite()
	{
		::IconSuiteRef result;
		ThrowOSStatus( ::NewIconSuite( &result ) );
		return nucleus::owned< IconSuiteRef >::seize( IconSuiteRef( result ) );
	}
	
	void DisposeIconSuite( nucleus::owned< IconSuiteRef > iconSuite )
	{
		ThrowOSStatus( ::DisposeIconSuite( iconSuite.release(), true ) );
	}
	
	void DisposeIconSuite( nucleus::owned< IconSuiteRef, DisposeIconSuiteButNotData > iconSuite )
	{
		ThrowOSStatus( ::DisposeIconSuite( iconSuite.release(), false ) );
	}
	
	void PlotIconSuite( const Rect&        rect,
	                    IconAlignmentType  align,
	                    IconTransformType  transform,
	                    IconSuiteRef       iconSuite )
	{
		ThrowOSStatus( ::PlotIconSuite( &rect, align, transform, iconSuite ) );
	}
	
	static void PlotIconHandle_Internal( const Rect&        area,
	                                     IconAlignmentType  align,
	                                     IconTransformType  transform,
	                                     Handle             icon )
	{
		ThrowOSStatus( ::PlotIconHandle( &area, align, transform, icon ) );
	}
	
	void PlotIconHandle( const Rect&        area,
	                     IconAlignmentType  align,
	                     IconTransformType  transform,
	                     PlainIconHandle    icon )
	{
		PlotIconHandle_Internal( area, align, transform, icon );
	}
	
	void PlotIconHandle( const Rect&        area,
	                     IconAlignmentType  align,
	                     IconTransformType  transform,
	                     MaskedIconHandle   icon )
	{
		PlotIconHandle_Internal( area, align, transform, icon );
	}
	
	// The plot sickens...
	
	void PlotSICNHandle( const Rect&        area,
	                     IconAlignmentType  align,
	                     IconTransformType  transform,
	                     SmallIconHandle    theSICN )
	{
		ThrowOSStatus( ::PlotSICNHandle( &area, align, transform, Handle( theSICN ) ) );
	}
	
	void PlotCIconHandle( const Rect&        rect,
	                      IconAlignmentType  align,
	                      IconTransformType  transform,
	                      CIconHandle        theCIcon )
	{
		ThrowOSStatus( ::PlotCIconHandle( &rect, align, transform, theCIcon ) );
	}
	
   nucleus::owned<IconRef> GetIconRef( Mac::FSVolumeRefNum vRefNum, Mac::FSCreator creator, Mac::FSType iconType )
     {
      IconRef result;
      ThrowOSStatus( ::GetIconRef( vRefNum, creator, iconType, &result ) );
      return nucleus::owned<IconRef>::seize( result );
     }

   GetIconRefFromFile_Result GetIconRefFromFile( const FSSpec& theFile )
     {
      IconRef icon;
      SInt16 label;
      ThrowOSStatus( ::GetIconRefFromFile( &theFile, &icon, &label ) );
      return GetIconRefFromFile_Result( nucleus::owned<IconRef>::seize( icon ),
                                        IconLabel( label ) );
     }

   nucleus::owned<IconRef> GetIconRef( Mac::FSCreator creator, Mac::FSType iconType )
     {
      return GetIconRef( kOnSystemDisk, creator, iconType );
     }

   nucleus::owned<IconRef> GetIconRef( Mac::FSType iconType )
     {
      return GetIconRef( kOnSystemDisk, kSystemIconsCreator, iconType );
     }

	nucleus::owned< IconRef > GetIconRefFromFolder( Mac::FSVolumeRefNum      vRefNum,
	                                                Mac::FSDirID             parentFolderID,
	                                                Mac::FSDirID             folderID,
	                                                Mac::FSIOFileAttributes  attributes,
	                                                Mac::FSUserPrivileges    accessPrivileges )
     {
      IconRef result;
      ThrowOSStatus( ::GetIconRefFromFolder( vRefNum,
                                             parentFolderID,
                                             folderID,
                                             attributes,
                                             accessPrivileges,
                                             &result ) );
      return nucleus::owned<IconRef>::seize( result );
     }

   GetIconRefFromFileInfo_Result GetIconRefFromFileInfo( const FSRef&           inRef,
                                                         UniCharCount           inFileNameLength,
                                                         const UniChar         *inFileName,
                                                         FSCatalogInfoBitmap    inWhichInfo,
                                                         const FSCatalogInfo&   inCatalogInfo,
                                                         IconServicesUsageFlags inUsageFlags )
     {
      IconRef icon;
      SInt16 label;
      ThrowOSStatus( ::GetIconRefFromFileInfo( &inRef,
                                               inFileNameLength,
                                               inFileName,
                                               inWhichInfo,
                                               &inCatalogInfo,
                                               inUsageFlags,
                                               &icon,
                                               &label ) );
      return GetIconRefFromFile_Result( nucleus::owned<IconRef>::seize( icon ),
                                        IconLabel( label ) );
     }

   GetIconRefFromFileInfo_Result GetIconRefFromFileInfo( const FSRef&           inRef,
                                                         UniCharCount           inFileNameLength,
                                                         const UniChar         *inFileName,
                                                         IconServicesUsageFlags inUsageFlags )
     {
      IconRef icon;
      SInt16 label;
      ThrowOSStatus( ::GetIconRefFromFileInfo( &inRef,
                                               inFileNameLength,
                                               inFileName,
                                               0,
                                               0,
                                               inUsageFlags,
                                               &icon,
                                               &label ) );
      return GetIconRefFromFile_Result( nucleus::owned<IconRef>::seize( icon ),
                                        IconLabel( label ) );
     }

   nucleus::owned<IconRef> RegisterIconRefFromFSRef( Mac::FSCreator creator, Mac::FSType iconType, const FSRef& iconFile )
     {
      IconRef result;
      ThrowOSStatus( ::RegisterIconRefFromFSRef( creator, iconType, &iconFile, &result ) );
      return nucleus::owned<IconRef>::seize( result );
     }

   nucleus::owned<IconRef> RegisterIconRefFromIconFile( Mac::FSCreator  creator,
                                                        Mac::FSType     iconType,
                                                        const FSSpec&   iconFile )
	  {
      IconRef result;
      ThrowOSStatus( ::RegisterIconRefFromIconFile( creator, iconType, &iconFile, &result ) );
      return nucleus::owned<IconRef>::seize( result );
     }
   
   void RegisterIconManagerErrors()
     {
      RegisterOSStatus< paramErr               >();
      RegisterOSStatus< memFullErr             >();
      RegisterOSStatus< memWZErr               >();
      RegisterOSStatus< resNotFound            >();
      RegisterOSStatus< noMaskFoundErr         >();
      RegisterOSStatus< invalidIconRefErr      >();
      RegisterOSStatus< noSuchIconErr          >();
      RegisterOSStatus< noIconDataAvailableErr >();
      RegisterOSStatus< afpIconTypeError       >();
     }
}

