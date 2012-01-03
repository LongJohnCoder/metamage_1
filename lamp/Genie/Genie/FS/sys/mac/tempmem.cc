/*
	Genie/FS/sys/mac/tempmem.cc
	---------------------------
*/

#include "Genie/FS/sys/mac/tempmem.hh"

// POSIX
#include <sys/stat.h>

// Nitrogen
#include "Nitrogen/MacMemory.hh"

// poseven
#include "poseven/types/errno_t.hh"

// Genie
#include "Genie/FS/FSTree.hh"
#include "Genie/IO/VirtualFile.hh"
#include "Genie/mmap/Handle_memory_mapping.hh"


namespace Genie
{
	
	namespace n = nucleus;
	namespace N = Nitrogen;
	namespace p7 = poseven;
	
	
	class TempMem_IOHandle : public VirtualFileHandle< IOHandle >
	{
		public:
			TempMem_IOHandle( const FSTreePtr&  file,
			                  OpenFlags         flags )
			:
				VirtualFileHandle< IOHandle >( file, flags )
			{
			}
			
			memory_mapping::intrusive_ptr Map( size_t length, off_t offset );
	};
	
	memory_mapping::intrusive_ptr TempMem_IOHandle::Map( size_t length, off_t offset )
	{
		if ( offset != 0 )
		{
			p7::throw_errno( EINVAL );
		}
		
		n::owned< N::Handle > h = N::TempNewHandle( length );
		
		return seize_ptr( new Handle_memory_mapping( h ) );
	}
	
	
	class FSTree_sys_mac_tempmem : public FSTree
	{
		public:
			FSTree_sys_mac_tempmem( const FSTreePtr&     parent,
			                        const plus::string&  name )
			:
				FSTree( parent, name, S_IFREG | 0 )
			{
			}
			
			off_t GetEOF() const  { return ::TempFreeMem(); }
			
			IOPtr Open( OpenFlags flags ) const;
	};
	
	
	IOPtr FSTree_sys_mac_tempmem::Open( OpenFlags flags ) const
	{
		return seize_ptr( new TempMem_IOHandle( Self(), flags ) );
		
	}
	
	FSTreePtr New_FSTree_sys_mac_tempmem( const FSTreePtr&     parent,
	                                      const plus::string&  name,
	                                      const void*          args )
	{
		return seize_ptr( new FSTree_sys_mac_tempmem( parent, name ) );
	}
	
}

