/*	=================
 *	SignalReceiver.cc
 *	=================
 */

#include "Genie/Process/SignalReceiver.hh"


namespace Genie
{
	
	SignalReceiver::SignalReceiver() : itsPendingSignals(),
	                                   itsBlockedSignals()
	{
	}
	
	SignalReceiver::SignalReceiver( const SignalReceiver& other )
	:
	  itsPendingSignals(),  // Reset pending signals on fork
	  itsBlockedSignals( other.itsBlockedSignals )
	{
	}
	
}

