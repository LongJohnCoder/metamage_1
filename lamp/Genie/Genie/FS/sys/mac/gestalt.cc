/*
	Genie/FS/sys/mac/gestalt.cc
	---------------------------
*/

#include "Genie/FS/sys/mac/gestalt.hh"

// Satndard C++
#include <algorithm>

// Mac OS
#ifndef __MACERRORS__
#include <MacErrors.h>
#endif

// plus
#include "plus/var_string.hh"

// MacScribe
#include "quad/utf8_quad_name.hh"

// poseven
#include "poseven/types/errno_t.hh"

// Nitrogen
#include "Mac/Toolbox/Utilities/ThrowOSStatus.hh"

#include "Nitrogen/Gestalt.hh"

// vfs
#include "vfs/dir_contents.hh"
#include "vfs/dir_entry.hh"

// Genie
#include "Genie/FS/basic_directory.hh"
#include "Genie/FS/FSTree.hh"
#include "Genie/FS/FSTree_Directory.hh"


namespace Nitrogen
{
	
	struct Gestalt_Record
	{
		Gestalt_Selector  selector;
		UInt32            zero;
	};
	
	typedef Gestalt_Record const *const *Gestalt_Handle;
	
	class GestaltTable_Container
	{
		private:
			Gestalt_Handle handle;
			
			// not implemented:
			GestaltTable_Container& operator=( const GestaltTable_Container& );
		
		public:
			typedef UInt16 size_type;
			typedef SInt16 difference_type;
			
			typedef Gestalt_Selector value_type;
			
			typedef const value_type &reference;
			typedef const value_type &const_reference;
			
			typedef const value_type *pointer;
			typedef const value_type *const_pointer;
			
			class const_iterator
			{
				friend class GestaltTable_Container;
				
				public:
					typedef GestaltTable_Container::size_type  size_type;
					typedef GestaltTable_Container::reference  reference;
					typedef GestaltTable_Container::pointer    pointer;
					
					typedef std::forward_iterator_tag iterator_category;
					
				private:
					Gestalt_Handle  handle;
					size_type       position;
					
					const_reference GetReference() const
					{
						return handle[0][ position ].selector;
					}
					
					void Advance()
					{
						++position;
						
						if ( GetReference() == 0x7fffffff )
						{
							position = 0xffff;
						}
					}
					
					const_iterator( Gestalt_Handle h, size_type pos ) : handle( h ), position( pos )
					{
					}
					
				public:
					const_iterator& operator++()      { Advance();  return *this; }
					const_iterator operator++(int)    { const_iterator old = *this; operator++(); return old; }
					
					reference operator*() const       { return GetReference(); }
					pointer operator->() const        { return &GetReference(); }
					
					friend bool operator==( const const_iterator& a, const const_iterator& b )    { return a.position == b.position; }
					friend bool operator!=( const const_iterator& a, const const_iterator& b )    { return !( a == b ); }
			};
			
			GestaltTable_Container( Gestalt_Handle h ) : handle( h )
			{
			}
			
			const_iterator begin() const              { return const_iterator( handle,      0 ); }
			const_iterator end() const                { return const_iterator( handle, 0xffff ); }
	};
	
	static GestaltTable_Container GestaltTable()
	{
		UInt32 tabl = Nitrogen::Gestalt( Nitrogen::GestaltSelector( 'tabl' ) );
		
		Gestalt_Handle h = reinterpret_cast< Gestalt_Handle >( tabl );
		
		if ( h == NULL  ||  *h == NULL )
		{
			Mac::ThrowOSStatus( nilHandleErr );
		}
		
		return GestaltTable_Container( h );
	}
	
}

namespace Genie
{
	
	namespace N = Nitrogen;
	
	
	using MacScribe::make_utf8_quad_name;
	using MacScribe::parse_utf8_quad_name;
	using MacScribe::quad_t;
	
	
	static bool is_valid_Gestalt_Selector( N::Gestalt_Selector selector )
	{
		try
		{
			N::Gestalt( selector );
		}
		catch ( ... )
		{
			return false;
		}
		
		return true;
	}
	
	static inline bool is_valid_Gestalt_Selector_name( const plus::string& name )
	{
		quad_t decoded;
		
		try
		{
			decoded = parse_utf8_quad_name( name );
		}
		catch ( ... )
		{
			return false;
		}
		
		return is_valid_Gestalt_Selector( N::Gestalt_Selector( decoded ) );
	}
	
	static FSTreePtr gestalt_lookup( const FSTree* parent, const plus::string& name )
	{
		if ( !is_valid_Gestalt_Selector_name( name ) )
		{
			poseven::throw_errno( ENOENT );
		}
		
		return Premapped_Factory( parent, name );
	}
	
	class gestalt_IteratorConverter
	{
		public:
			vfs::dir_entry operator()( N::Gestalt_Selector selector ) const
			{
				const bool valid = is_valid_Gestalt_Selector( selector );
				
				const ino_t inode = 0;
				
				plus::var_string name;
				
				if ( !valid )
				{
					name += '.';
				}
				
				name += make_utf8_quad_name( selector );
				
				return vfs::dir_entry( inode, name );
			}
	};
	
	static void gestalt_iterate( const FSTree* parent, vfs::dir_contents& cache )
	{
		gestalt_IteratorConverter converter;
		
		N::GestaltTable_Container sequence = N::GestaltTable();
		
		std::transform( sequence.begin(),
		                sequence.end(),
		                std::back_inserter( cache ),
		                converter );
	}
	
	FSTreePtr New_FSTree_sys_mac_gestalt( const FSTree*        parent,
	                                      const plus::string&  name,
	                                      const void*          args )
	{
		return new_basic_directory( parent, name, gestalt_lookup, gestalt_iterate );
	}
	
}

