/*	===================
 *	MacBinaryDecoder.hh
 *	===================
 */

#ifndef MACBINARYDECODER_HH
#define MACBINARYDECODER_HH

// Nitrogen
#ifndef NITROGEN_APPLEEVENTS_HH
#include "Nitrogen/AppleEvents.hh"
#endif

// MacFiles
#include "MacFiles/Classic.hh"
#include "MacFiles/Unicode.hh"

// Pedestal
#ifndef PEDESTAL_APPLICATION_HH
#include "Pedestal/Application.hh"
#endif


namespace MacBinaryDecoder
{
	
	struct FSSpec_Io_Details : public Nitrogen::FSSpec_Io_Details
	{
		static const Mac::DescType typeFileSpec = Mac::typeFSS;
	};
	
	struct FSRef_Io_Details : public Nitrogen::FSRef_Io_Details
	{
		static const Mac::DescType typeFileSpec = Mac::typeFSRef;
	};
	
#if TARGET_API_MAC_CARBON
	
	typedef FSRef_Io_Details Io_Details;
	
#else
	
	typedef FSSpec_Io_Details Io_Details;
	
#endif
	
	
	class App : public Pedestal::Application
	{
		private:
			static App* theApp;
			
			nucleus::owned< Nitrogen::AEEventHandler > itsOpenDocsEventHandler;
		
		public:
			static App& Get();
			
			App();
			~App();
			
			void OpenDocument( const Io_Details::file_spec& file );
	};
	
}

#endif

