/*	==================
 *	CFMLateImporter.hh
 *	==================
 */

#ifndef CFMLATEIMPORTER_HH
#define CFMLATEIMPORTER_HH

// Nitrogen
#include "Nitrogen/CodeFragments.h"


namespace CFMLateImporter
{
	
	void ImportLibrary( ConstStr255Param                     weakLinkedLibraryName,
	                    Nucleus::Owned< CFragConnectionID >  connIDToImport );
	
}

#endif

