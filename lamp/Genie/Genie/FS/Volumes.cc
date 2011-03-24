/*
	Volumes.cc
	----------
*/

#include "Genie/FS/Volumes.hh"

// POSIX
#include <sys/stat.h>

// Mac OS
#ifndef __FILES__
#include <Files.h>
#endif

// plus
#include "plus/mac_utf8.hh"
#include "plus/replaced_string.hh"
#include "plus/var_string.hh"

// Nitrogen
#include "Mac/Files/Types/FSDirSpec.hh"
#include "Mac/Files/Types/FSVolumeRefNum.hh"
#include "Mac/Toolbox/Utilities/ThrowOSStatus.hh"

#include "Nitrogen/Str.hh"

// Genie
#include "Genie/FS/FSSpec.hh"
#include "Genie/FS/FSTreeCache.hh"
#include "Genie/FS/ResolvableSymLink.hh"


namespace Genie
{
	
	namespace n = nucleus;
	namespace N = Nitrogen;
	
	
	static inline plus::string slashes_from_colons( const plus::string& unix_name )
	{
		return plus::replaced_string( unix_name, ':', '/' );
	}
	
	static inline plus::string colons_from_slashes( const plus::string& mac_name )
	{
		return plus::replaced_string( mac_name, '/', ':' );
	}
	
	static inline plus::string colons_from_slashes( const unsigned char* mac_name )
	{
		return colons_from_slashes( plus::string( mac_name ) );
	}
	
	
	static Mac::FSVolumeRefNum GetVRefNum( const plus::string& name )
	{
		N::Str255 name_copy = name;
		
		HParamBlockRec pb;
		
		pb.volumeParam.ioVRefNum  = 0;
		pb.volumeParam.ioNamePtr  = name_copy;
		pb.volumeParam.ioVolIndex = -1;  // use use ioNamePtr/ioVRefNum combination
		
		Mac::ThrowOSStatus( ::PBHGetVInfoSync( &pb ) );
		
		return Mac::FSVolumeRefNum( pb.volumeParam.ioVRefNum );
	}
	
	
	class FSTree_Volumes : public FSTree
	{
		public:
			FSTree_Volumes( const FSTreePtr&     parent,
			                const plus::string&  name )
			:
				FSTree( parent, name, S_IFDIR | 0700 )
			{
			}
			
			ino_t Inode() const  { return fsRtParID; }
			
			FSTreePtr Lookup_Child( const plus::string& name, const FSTree* parent ) const;
			
			void IterateIntoCache( FSTreeCache& cache ) const;
	};
	
	
	FSTreePtr New_FSTree_Volumes( const FSTreePtr&     parent,
	                              const plus::string&  name,
	                              const void*          args )
	{
		return seize_ptr( new FSTree_Volumes( parent, name ) );
	}
	
	
	class FSTree_Volumes_Link : public FSTree_ResolvableSymLink
	{
		public:
			FSTree_Volumes_Link( const FSTreePtr&     parent,
			                     const plus::string&  name )
			:
				FSTree_ResolvableSymLink( parent, name )
			{
			}
			
			FSTreePtr ResolveLink() const;
	};
	
	FSTreePtr FSTree_Volumes_Link::ResolveLink() const
	{
		// Convert UTF-8 to MacRoman, ':' to '/'
		plus::var_string mac_name = slashes_from_colons( plus::mac_from_utf8( Name() ) );
		
		mac_name += ":";
		
		const Mac::FSVolumeRefNum vRefNum = GetVRefNum( mac_name );
		
		const Mac::FSDirSpec dir = n::make< Mac::FSDirSpec >( vRefNum, Mac::fsRtDirID );
		
		return FSTreeFromFSDirSpec( dir, VolumeIsOnServer( vRefNum ) );
	}
	
	
	FSTreePtr FSTree_Volumes::Lookup_Child( const plus::string& name, const FSTree* parent ) const
	{
		return seize_ptr( new FSTree_Volumes_Link( (parent ? parent : this)->Self(), name ) );
	}
	
	void FSTree_Volumes::IterateIntoCache( FSTreeCache& cache ) const
	{
		for ( int i = 1;  true;  ++i )
		{
			Str27 mac_name = "\p";
			
			HParamBlockRec pb;
			
			pb.volumeParam.ioNamePtr  = mac_name;
			pb.volumeParam.ioVRefNum  = 0;
			pb.volumeParam.ioVolIndex = i;
			
			const OSErr err = ::PBHGetVInfoSync( &pb );
			
			if ( err == nsvErr )
			{
				break;
			}
			
			Mac::ThrowOSStatus( err );
			
			const ino_t inode = -pb.volumeParam.ioVRefNum;
			
			const plus::string name = plus::utf8_from_mac( colons_from_slashes( mac_name ) );
			
			cache.push_back( FSNode( inode, name ) );
		}
	}
	
}

