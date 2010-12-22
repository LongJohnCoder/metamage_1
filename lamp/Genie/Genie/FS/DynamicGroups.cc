/*	================
 *	DynamicGroups.cc
 *	================
 */

#include "Genie/FS/DynamicGroups.hh"

// iota
#include "iota/decimal.hh"

// poseven
#include "poseven/types/errno_t.hh"

// Genie
#include "Genie/FS/FSTreeCache.hh"


namespace Genie
{
	
	FSTree_Dynamic_N::FSTree_Dynamic_N( const FSTreePtr&      parent,
	                                    const plus::string&   name,
	                                    DynamicElementGetter  getter )
	:
		FSTree( parent, name ),
		itsGetter( getter )
	{
	}
	
	boost::shared_ptr< IOHandle > FSTree_Dynamic_N::Open( OpenFlags flags ) const
	{
		const unsigned id = iota::parse_unsigned_decimal( Name().c_str() );
		
		return itsGetter( id );
	}
	
	class DynamicGroup_IteratorConverter
	{
		public:
			FSNode operator()( const DynamicGroup::value_type& value ) const
			{
				const unsigned id = value.first;
				
				const ino_t inode = id;
				
				const plus::string name = iota::inscribe_unsigned_decimal( id );
				
				return FSNode( inode, name );
			}
	};
	
	FSTreePtr FSTree_DynamicGroup_Base::Lookup_Child( const plus::string& name, const FSTree* parent ) const
	{
		const unsigned id = iota::parse_unsigned_decimal( name.c_str() );
		
		const DynamicGroup& sequence = ItemSequence();
		
		if ( sequence.find( id ) == sequence.end() )
		{
			poseven::throw_errno( ENOENT );
		}
		
		return seize_ptr( new FSTree_Dynamic_N( (parent ? parent : this)->Self(), name, Getter() ) );
	}
	
	void FSTree_DynamicGroup_Base::IterateIntoCache( FSTreeCache& cache ) const
	{
		DynamicGroup_IteratorConverter converter;
		
		const DynamicGroup& sequence = ItemSequence();
		
		std::transform( sequence.begin(),
		                sequence.end(),
		                std::back_inserter( cache ),
		                converter );
	}
	
}

