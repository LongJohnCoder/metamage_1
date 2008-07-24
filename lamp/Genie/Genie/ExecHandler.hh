/*	====================
 *	Genie/ExecHandler.hh
 *	====================
 */

#ifndef GENIE_EXECHANDLER_HH
#define GENIE_EXECHANDLER_HH

// Nitrogen
#include "Nitrogen/AppleEvents.h"
#include "Nitrogen/Files.h"


namespace Genie
{
	
	class GenieExecHandler : public Nucleus::Owned< Nitrogen::AEEventHandler >
	{
		public:
			static void AppleEventHandler( const Nitrogen::AppleEvent& appleEvent, Nitrogen::AppleEvent& reply, GenieExecHandler* handler );
			
			GenieExecHandler();
			
			void HandleAppleEvent( const Nitrogen::AppleEvent& appleEvent, Nitrogen::AppleEvent& outReply );
			
			int ExecArgList( const std::vector< std::string >& argVec );
			int ExecString( const std::string& cmd, const Nitrogen::FSDirSpec& cwd );
	};
	
}

#endif

