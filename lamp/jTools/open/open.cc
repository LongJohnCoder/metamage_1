/*	=======
 *	open.cc
 *	=======
 */

// Universal Interfaces
#ifndef __AEREGISTRY__
#include <AERegistry.h>
#endif

// C Standard Library
#include <stdlib.h>

// C++ Standard Library
#include <functional>
#include <string>
#include <vector>

// Nucleus
#include "Nucleus/Exception.h"
#include "Nucleus/TheExceptionBeingHandled.h"

// Nitrogen
#include "Nitrogen/AEInteraction.h"
#include "Nitrogen/Str.h"

// POSeven
#include "POSeven/Errno.hh"

// Nitrogen Extras / Iteration
#include "Iteration/AEDescListItems.h"

// Nitrogen Extras / Operators
//#include "Operators/AEDataModel.h"

// Nitrogen Extras / Utilities
#include "Utilities/Processes.h"

// Divergence
#include "Divergence/Utilities.hh"

// Orion
#include "Orion/GetOptions.hh"
#include "Orion/Main.hh"


namespace N = Nitrogen;
namespace NN = Nucleus;
namespace p7 = poseven;
namespace NX = NitrogenExtras;
namespace Div = Divergence;
namespace O = Orion;


enum
{
	sigFinder       = 'MACS',
	sigBBEdit       = 'R*ch',
	sigTextWrangler = '!Rch',
	
	// FIXME
	sigGoodTextEditor = TARGET_API_MAC_CARBON ? sigTextWrangler : sigBBEdit
};

static FSSpec ResolvePathname( const std::string& pathname, bool macPathname )
{
	return macPathname ? N::FSMakeFSSpec         ( N::Str255( pathname       ) ) 
	                   : Div::ResolvePathToFSSpec(            pathname.c_str() );
}

static NN::Owned< N::AEDesc_Data > CoerceFSSpecToAliasDesc( const FSSpec& item )
{
	return N::AECoercePtr< N::typeFSS >( item, N::typeAlias );
}

static NN::Owned< N::AppleEvent > MakeOpenDocsEvent( const std::vector< FSSpec >&  items,
                                                     const ProcessSerialNumber&    psn )
{
	NN::Owned< N::AEDescList_Data > documents = N::AECreateList< N::AEDescList_Data >( false );
	
	std::transform( items.begin(),
	                items.end(),
	                N::AEDescList_Item_BackInserter( documents ),
	                std::ptr_fun( CoerceFSSpecToAliasDesc ) );
	
	//using namespace NN::Operators;
	
	NN::Owned< N::AppleEvent > appleEvent = N::AECreateAppleEvent( N::kCoreEventClass,
	                                                               N::kAEOpenDocuments,
	                                                               N::AECreateDesc< N::AEDesc_Data, N::typeProcessSerialNumber >( psn ) );
	
	N::AEPutParamDesc( appleEvent, N::keyDirectObject, documents );
	
	return appleEvent;
}

static void OpenItemsWithRunningApp( const std::vector< FSSpec >& items, const ProcessSerialNumber& psn )
{
	N::AESend( MakeOpenDocsEvent( items, psn ),
	           N::kAENoReply | N::kAECanInteract );
}

static void LaunchApplicationWithDocsToOpen( const FSSpec& app, const std::vector< FSSpec >& items )
{
	std::auto_ptr< AppParameters > appParameters
		= N::AEGetDescData< N::typeAppParameters >( N::AECoerceDesc( MakeOpenDocsEvent( items, N::NoProcess() ),
	                                                                 N::typeAppParameters ) );
	
	N::LaunchApplication( app, N::LaunchFlags(), appParameters.get() );
}


static std::string gAppNameToOpenIn;
static std::string gAppSigToOpenIn;

static bool gOpenInEditor    = false;
static bool gUseMacPathnames = false;

static void DefineOptions()
{
	O::BindOption( "--app",  gAppNameToOpenIn );
	O::BindOption( "--sig",  gAppSigToOpenIn  );
	O::BindOption( "--mac",  gUseMacPathnames );
	O::BindOption( "--edit", gOpenInEditor    );
	
	O::AliasOption( "--app",  "-a" );
	O::AliasOption( "--edit", "-e" );
	O::AliasOption( "--edit", "-t" );
}

static N::OSType SignatureOfAppForOpening()
{
	if ( gOpenInEditor )
	{
		// User has specified default text editor.
		// Check MAC_EDITOR_SIGNATURE environment variable first.
		// If set, it must be a four-character code.
		
		if ( const char* macEditorSignature = getenv( "MAC_EDITOR_SIGNATURE" ) )
		{
			return NN::Convert< N::OSType, std::string >( macEditorSignature );
		}
		
		// No MAC_EDITOR_SIGNATURE; default to BBEdit/TextWrangler.
		// FIXME:  We could be smarter about this...
		
		return N::OSType( sigGoodTextEditor );
	}
	
	if ( !gAppSigToOpenIn.empty() )
	{
		// User has specified an application by its signature
		return NN::Convert< N::OSType >( gAppSigToOpenIn );
	}
	
	// Otherwise, give everything to the Finder.
	
	return N::OSType( sigFinder );
}

static void OpenItemsUsingOptions( const std::vector< FSSpec >& items )
{
	// we either have a pathname or signature for the app.
	// if pathname, resolve to FSSpec and check if it's running.
	// if running, send AE and return
	// if signature, check if it's running.
	// if it's running, send it odoc and return
	// if not, lookup in DT
	// launch with app parameters
	
	// ways to specify app:
	// --app: pathname
	// --sig: signature
	// -e or -t: either?
	// default: sig
	
	FSSpec appFile;
	
	if ( !gAppNameToOpenIn.empty() )
	{
		// User has specified an application by its pathname
		
		// Resolve to FSSpec
		appFile = Div::ResolvePathToFSSpec( gAppNameToOpenIn.c_str() );
		
		try
		{
			// Find it if running
			ProcessSerialNumber psn = NX::FindProcess( appFile );
			
			// The app is already running -- send it an odoc event
			OpenItemsWithRunningApp( items, psn );
			
			// We're done
			return;
		}
		catch ( const N::ProcNotFound& err )
		{
		#ifdef __MWERKS__
			
			if ( err.Get() != procNotFound )
			{
				throw;
			}
			
		#endif
			
			// No such process
			// appFile is already set
		}
	}
	else
	{
		// Look up by signature.
		
		// Pick a signature
		N::OSType signature = SignatureOfAppForOpening();
		
		try
		{
			// Find it if running
			ProcessSerialNumber psn = NX::FindProcess( signature );
			
			// The app is already running -- send it an odoc event
			OpenItemsWithRunningApp( items, psn );
			
			// We're done
			return;
		}
		catch ( const N::ProcNotFound& err )
		{
		#ifdef __MWERKS__
			
			if ( err.Get() != procNotFound )
			{
				throw;
			}
			
		#endif
			
			// No such process
			appFile = N::DTGetAPPL( signature );
		}
	}
	
	LaunchApplicationWithDocsToOpen( appFile, items );
}

int O::Main( int argc, argv_t argv )
{
	NN::RegisterExceptionConversion< NN::Exception, N::OSStatus >();
	
	DefineOptions();
	
	O::GetOptions( argc, argv );
	
	char const *const *freeArgs = O::FreeArguments();
	
	std::vector< FSSpec > itemsToOpen;
	
	for ( char const *const *it = freeArgs;  *it != NULL;  ++it )
	{
		const char* pathname = *it;
		
		try
		{
			FSSpec item = ResolvePathname( pathname, gUseMacPathnames );
			
			if ( !io::item_exists( item ) )
			{
				throw p7::enoent();
			}
			
			itemsToOpen.push_back( item );
		}
		catch ( const N::OSStatus& err )
		{
			std::fprintf( stderr, "open: %s: OSStatus %d\n", pathname, err.Get() );
		}
		catch ( const p7::errno_t& errnum )
		{
			std::fprintf( stderr, "open: %s: %s\n", pathname, std::strerror( errnum ) );
		}
	}
	
	OpenItemsUsingOptions( itemsToOpen );
	
	return 0;
}

