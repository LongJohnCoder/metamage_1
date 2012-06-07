/*	========================
 *	Handle_memory_mapping.cc
 *	========================
 */

#include "Genie/mmap/Handle_memory_mapping.hh"

// Nitrogen
#ifndef NITROGEN_MACMEMORY_HH
#include "Nitrogen/MacMemory.hh"
#endif


namespace Genie
{
	
	namespace N = Nitrogen;
	
	
	Handle_memory_mapping::Handle_memory_mapping( nucleus::owned< N::Handle > h )
	:
		memory_mapping( *h.get() ),
		its_handle( h )
	{
		N::HLock( its_handle );
	}
	
}

