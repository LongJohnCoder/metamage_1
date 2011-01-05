/*	================
 *	OpenTransport.hh
 *	================
 */

#ifndef CLASSICTOOLBOX_OPENTRANSPORT_HH
#define CLASSICTOOLBOX_OPENTRANSPORT_HH

// Nostalgia
#include "Nostalgia/OpenTransport.hh"

// Nitrogen
#ifndef NITROGEN_OPENTRANSPORT_HH
#include "Nitrogen/OpenTransport.hh"
#endif


namespace Nitrogen
{
	
	inline void InitOpenTransport()
	{
		Mac::ThrowOSStatus( ::InitOpenTransport() );
	}
	
	using ::CloseOpenTransport;
	
	inline nucleus::owned< EndpointRef >
	//
	OTOpenEndpoint( nucleus::owned< OTConfigurationRef >  config,
	                TEndpointInfo*                        info = NULL )
	{
		::OSStatus err;
		
		EndpointRef result = ::OTOpenEndpoint( config.release(),
		                                       OTOpenFlags( 0 ),
		                                       info,
		                                       &err );
		
		Mac::ThrowOSStatus( err );
		
		return nucleus::owned< EndpointRef >::seize( result );
	}
	
}

#endif

