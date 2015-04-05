// Nitrogen/Scrap.hh
// -----------------
//
// Maintained by Joshua Juran

// Part of the Nitrogen project.
//
// Written 2004-2006 by Joshua Juran.
//
// This code was written entirely by the above contributor, who places it
// in the public domain.


#ifndef NITROGEN_SCRAP_HH
#define NITROGEN_SCRAP_HH

// Mac OS X
#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

// Mac OS
#ifndef __SCRAP__
#include <Scrap.h>
#endif

// nucleus
#ifndef NUCLEUS_ERRORSREGISTERED_HH
#include "nucleus/errors_registered.hh"
#endif

// Replace macro

#ifdef kScrapRefNone
#undef kScrapRefNone

static const ScrapRef kScrapRefNone = ScrapRef( NULL );

#endif

namespace Nitrogen
{
	
	NUCLEUS_DECLARE_ERRORS_DEPENDENCY( ScrapManager );
	
#if CALL_NOT_IN_CARBON
	
	void ZeroScrap();
	
#endif
	
	ScrapRef GetCurrentScrap();
	
	void ClearCurrentScrap();
	
}

#endif
