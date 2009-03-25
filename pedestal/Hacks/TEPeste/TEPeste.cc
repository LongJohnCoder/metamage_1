/*
	
	TEPeste
	
	Joshua Juran
	
*/

// Universal Interfaces
#include <Events.h>
#include <Resources.h>
#include <Sound.h>
#include <TextEdit.h>
#include <OSUtils.h>

// Standard C/C++
#include <cstring>

// Silver
#include "Silver/Install.hh"
#include "Silver/Patch.hh"
#include "Silver/Procs.hh"
#include "Silver/Traps.hh"


#pragma exceptions off


namespace Ag = Silver;

using namespace Ag::Trap_ProcPtrs;


static const char* gGags[] =
{
	"Boo!",
	"Whazzuuup!",
	"Nobody expects the Spanish Inquisition!",
	"You REALLY ought to get some sleep...",
	"� Vote for Joshua Juran �",
	""
};

static const unsigned kCountOfGags = sizeof gGags / sizeof gGags[0] - 1;

static bool gLastTimeWasAGag;


inline bool ShouldDoAGag()
{
	return !gLastTimeWasAGag  &&  (TickCount() & 3) == 0;
}

inline const char* PickAGag( short which )
{
	return gGags[ which ];
}

static void Payload( TEHandle hTE )
{
	const char* gag = PickAGag( TickCount() % kCountOfGags );
	
	long len = std::strlen( gag );
	long start = hTE[0]->selStart;
	
	SysBeep( 30 );
	
	TEDelete( hTE );
	
	TEInsert( gag, len, hTE );
	
	TESetSelect( start, start + len, hTE );
	
	UInt32 finalTicks;
	
	::Delay( 30, &finalTicks );
}

namespace
{
	
	void PatchedTEPaste( TEHandle hTE, TEPasteProcPtr nextHandler )
	{
		if ( (gLastTimeWasAGag = ShouldDoAGag()) )
		{
			Payload( hTE );
		}
		
		nextHandler( hTE );
	}
	
}

static OSErr Installer()
{
	Ag::TrapPatch< _TEPaste, PatchedTEPaste >::Install();
	
	return noErr;
}

int main()
{
	return Ag::Install( Installer );
}

