/*	===================
 *	CRMSerialDevices.hh
 *	===================
 */

#ifndef CLASSICTOOLBOX_CRMSERIALDEVICES_HH
#define CLASSICTOOLBOX_CRMSERIALDEVICES_HH

// Mac OS
#ifndef __CRMSERIALDEVICES__
#include <CRMSerialDevices.h>
#endif

// Nitrogen
#ifndef NITROGEN_ICONS_HH
#include "Nitrogen/Icons.hh"
#endif
#ifndef NITROGEN_MACMEMORY_HH
#include "Nitrogen/MacMemory.hh"
#endif

// Nitrogen Extras / ClassicToolbox
#ifndef CLASSICTOOLBOX_COMMRESOURCES_HH
#include "ClassicToolbox/CommResources.hh"
#endif


namespace Nitrogen
{
	
	using ::CRMIconRecord;
	using ::CRMIconPtr;
	using ::CRMIconHandle;
	
	using ::CRMSerialRecord;
	using ::CRMSerialPtr;
	
	static const CRMDeviceType crmSerialDevice = CRMDeviceType( ::crmSerialDevice );
	
	template <> struct CRMAttributes_Traits< crmSerialDevice >  { typedef CRMSerialPtr Type; };
	
}

namespace nucleus
{
	
	template <> struct disposer< CRMIconHandle >
	{
		typedef CRMIconHandle  argument_type;
		typedef void           result_type;
		
		void operator()( CRMIconHandle deviceIcon ) const
		{
			Nitrogen::IconSuiteRef iconSuite = (*deviceIcon)->theSuite;
			
			disposer< Nitrogen::IconSuiteRef >()( iconSuite  );
			disposer< Nitrogen::Handle       >()( deviceIcon );
		}
	};
	
	template <> struct disposer< CRMSerialPtr >
	{
		typedef CRMSerialPtr  argument_type;
		typedef void          result_type;
		
		void operator()( CRMSerialPtr crmSerial ) const
		{
			disposer< Nitrogen::Handle >()( crmSerial->inputDriverName  );
			disposer< Nitrogen::Handle >()( crmSerial->outputDriverName );
			disposer< Nitrogen::Handle >()( crmSerial->name             );
			disposer< CRMIconHandle    >()( crmSerial->deviceIcon       );
			disposer< Nitrogen::Ptr    >()( crmSerial                   );
		}
	};
	
}

namespace Nitrogen
{
	
	inline Str255 GetCRMSerialName( CRMSerialPtr crmSerial )  { return *crmSerial->name; }
	
	nucleus::owned< CRMSerialPtr > New_CRMSerialRecord( ConstStr255Param inputDriverName,
	                                                    ConstStr255Param outputDriverName,
	                                                    ConstStr255Param portName );
	
	class CRMSerialDevice_Container : public CRMResource_Container
	{
		friend CRMSerialDevice_Container CRMSerialDevices();
		
		private:
			CRMSerialDevice_Container() : CRMResource_Container( crmSerialDevice )  {}
	};
	
	inline CRMSerialDevice_Container CRMSerialDevices()
	{
		return CRMSerialDevice_Container();
	}
	
}

#endif

