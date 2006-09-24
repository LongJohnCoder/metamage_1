/*	=====================
 *	ApplicationContext.cc
 *	=====================
 */

#include "Pedestal/ApplicationContext.hh"

// Nitrogen
#include "Nitrogen/Processes.h"

// Nitrogen Extras / Utilities
#include "Utilities/Files.h"


namespace Pedestal
{
	
	namespace N = Nitrogen;
	
	MacToolboxInit::MacToolboxInit()
	{
	#if !TARGET_API_MAC_CARBON
		
		::InitGraf( &qd.thePort );
		::InitFonts();
		::InitWindows();
		::InitMenus();
		::TEInit();
		::InitDialogs( NULL );
		
	#endif
		
		::InitCursor();
		// FlushEvents?
	}
	
	MemoryInit::MemoryInit( std::size_t moreMasters )
	{
	#if !TARGET_API_MAC_CARBON
		
		::MaxApplZone();
		
	#endif
		
		for ( std::size_t i = 0;  i < moreMasters;  ++i )
		{
			::MoreMasters();
		}
	}
	
	static VersRec ReadVersion()
	{
		// Read our version resource.
		try
		{
			return **N::Handle_Cast< VersRec >( N::Get1Resource( N::ResType( 'vers' ), N::ResID( 1 ) ) );
		}
		catch ( N::ResNotFound )
		{
			// Return below
		}
		
		return VersRec();
	}
	
	ApplicationContext::ApplicationContext()
	:
		fMemoryInit   ( 0 ), 
		fResFileRefNum( N::CurResFile() ), 
		fLaunchDir    ( N::FSpGetParent( N::GetProcessAppSpec( N::CurrentProcess() ) ) ), 
		fVersion      ( ReadVersion() )
	{
	}
	
}

