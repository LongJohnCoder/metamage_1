/*	===================
 *	MacBinaryDecoder.hh
 *	===================
 */

#ifndef MACBINARYDECODER_HH
#define MACBINARYDECODER_HH

// Nucleus
#include "Nucleus/Owned.h"

// MacFiles
#include "MacFiles.hh"
#include "UnicodeMacFiles.hh"

// Pedestal
#include "Pedestal/Application.hh"
#include "Pedestal/AboutBox.hh"


namespace MacBinaryDecoder
{
	
	struct FSSpec_Io_Details : public Nitrogen::FSSpec_Io_Details
	{
		static const Nitrogen::DescType typeFileSpec = Nitrogen::typeFSS;
	};
	
	struct FSRef_Io_Details : public Nitrogen::FSRef_Io_Details
	{
		static const Nitrogen::DescType typeFileSpec = Nitrogen::typeFSRef;
	};
	
#if TARGET_API_MAC_CARBON
	
	typedef FSRef_Io_Details Io_Details;
	
#else
	
	typedef FSSpec_Io_Details Io_Details;
	
#endif
	
	
	class App : public Pedestal::Application,
	            public Pedestal::AboutBoxOwner
	{
		private:
			static App* theApp;
			
			Pedestal::AboutHandler< App > itsAboutHandler;
			Nucleus::Owned< Nitrogen::AEEventHandler > itsOpenDocsEventHandler;
		
		public:
			static App& Get();
			
			App();
			~App();
			
			void OpenDocument( const Io_Details::file_spec& file );
	};
	
}

#endif

