/*	===============
 *	AEObjectModel.h
 *	===============
 */

#ifndef AEOBJECTMODEL_AEOBJECTMODEL_H
#define AEOBJECTMODEL_AEOBJECTMODEL_H

#ifndef __AEREGISTRY__
#include <AERegistry.h>
#endif

#ifndef NITROGEN_AEOBJECTS_HH
#include "Nitrogen/AEObjects.hh"
#endif


namespace Nitrogen
{
	
	inline nucleus::owned< AEDesc_Token > GetRootToken()
	{
		return AEInitializeDesc< AEDesc_Token >();
	}
	
	UInt32 ComputeAbsoluteIndex( const AEDesc_Data&  keyData,
	                             std::size_t         count );
	
	pascal OSErr OSLCompare( ::DescType     op,
	                         const AEDesc*  obj1,
	                         const AEDesc*  obj2,
	                         ::Boolean*     result );
	
	pascal OSErr OSLCount( ::DescType     desiredClass,
	                       ::DescType     containerClass,
	                       const AEDesc*  containerToken,
	                       SInt32*        result );
	
	pascal OSErr OSLDisposeToken( AEDesc* token );
	
	void AESetObjectCallbacks();
	
	nucleus::owned< AEDesc_Token > DispatchPropertyAccess( Mac::AEObjectClass   desiredClass,
	                                                       const AEDesc_Token&  containerToken,
	                                                       Mac::AEObjectClass   containerClass,
	                                                       Mac::AEKeyForm       keyForm,
	                                                       const AEDesc_Data&   keyData,
	                                                       RefCon );
	
	nucleus::owned< AEDesc_Token > DispatchAccessToList( Mac::AEObjectClass   desiredClass,
	                                                     const AEDesc_Token&  containerToken,
	                                                     Mac::AEObjectClass   containerClass,
	                                                     Mac::AEKeyForm       keyForm,
	                                                     const AEDesc_Data&   keyData,
	                                                     RefCon );
	
}

#endif
