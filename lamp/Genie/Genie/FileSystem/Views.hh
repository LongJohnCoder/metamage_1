/*	========
 *	Views.hh
 *	========
 */

#ifndef GENIE_FILESYSTEM_VIEWS_HH
#define GENIE_FILESYSTEM_VIEWS_HH

// Standard C++
#include <memory>

// Genie
#include "Genie/FileSystem/FSTree_Directory.hh"
#include "Genie/FileSystem/Scribes.hh"


struct Rect;

namespace Pedestal
{
	
	class View;
	
}

namespace Genie
{
	
	void RemoveAllViewParameters( const FSTree* parent );
	
	
	typedef boost::shared_ptr< Pedestal::View > (*ViewFactory)( const FSTree* delegate );
	
	
	bool InvalidateWindowForView( const FSTree* view );
	
	
	template < class Scribe, typename Scribe::Value& (*Access)( const FSTree* ) >
	struct View_Property
	{
		static std::string Get( const FSTree* that, bool binary )
		{
			return Freeze< Scribe >( Access( GetViewKey( that ) ), binary );
		}
		
		static void Set( const FSTree* that, const char* begin, const char* end, bool binary )
		{
			const FSTree* view = GetViewKey( that );
			
			Access( view ) = Vivify< Scribe >( begin, end, binary );
			
			InvalidateWindowForView( view );
		}
	};
	
	
	class FSTree_new_View : public FSTree
	{
		private:
			typedef const FSTree_Premapped::Mapping* Mappings;
			
			typedef void (*Destructor)( const FSTree* delegate );
			
			ViewFactory  itsFactory;
			Mappings     itsMappings;
			Destructor   itsDestructor;
			
			virtual FSTreePtr CreateDelegate( const FSTreePtr&    parent,
			                                  const std::string&  name ) const;
		
		public:
			FSTree_new_View( const FSTreePtr&    parent,
			                 const std::string&  name,
			                 ViewFactory         factory,
			                 Mappings            mappings,
			                 Destructor          dtor ) : FSTree( parent, name ),
			                                              itsFactory( factory ),
			                                              itsMappings( mappings ),
			                                              itsDestructor( dtor )
			{
			}
			
			void HardLink( const FSTreePtr& target ) const;
	};
	
	inline const FSTree* GetViewKey( const FSTree* that )  { return that->ParentRef().get(); }
	
	class FSTree_View : public FSTree
	{
		private:
			typedef boost::shared_ptr< Pedestal::View >& (*ViewGetter)( const FSTree* );
			
			ViewGetter itsGetter;
		
		public:
			FSTree_View( const FSTreePtr&    parent,
			             const std::string&  name,
			             ViewGetter          get = NULL )
			:
				FSTree( parent, name ),
				itsGetter( get )
			{
			}
			
			const FSTree* ParentKey() const  { return ParentRef().get(); }
			
			bool IsDirectory() const  { return Exists(); }
			
			bool Exists() const;
			
			void SetTimes() const;
			
			void Delete() const;
			
			void CreateDirectory( mode_t mode ) const;
			
			FSTreePtr Lookup( const std::string& name ) const;
			
			FSIteratorPtr Iterate() const;
			
			virtual boost::shared_ptr< Pedestal::View >& Get() const
			{
				ASSERT( itsGetter != NULL );
				
				return itsGetter( ParentKey() );
			}
	};
	
	
	template < FSTree_View::ViewGetter getter >
	class FSTree_X_view : public FSTree_View
	{
		public:
			FSTree_X_view( const FSTreePtr&    parent,
			               const std::string&  name ) : FSTree_View( parent, name, getter )
			{
			}
	};
	
}

#endif

