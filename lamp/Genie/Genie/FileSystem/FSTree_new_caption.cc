/*	=====================
 *	FSTree_new_caption.cc
 *	=====================
 */

#include "Genie/FileSystem/FSTree_new_caption.hh"

// POSIX
#include <fcntl.h>

// Pedestal
#include "Pedestal/Caption.hh"

// Genie
#include "Genie/FileSystem/FSTree_Directory.hh"
#include "Genie/FileSystem/FSTree_Property.hh"
#include "Genie/FileSystem/FSTree_sys_window_REF.hh"
#include "Genie/FileSystem/Views.hh"
#include "Genie/IO/VirtualFile.hh"


namespace Genie
{
	
	namespace N = Nitrogen;
	namespace NN = Nucleus;
	namespace Ped = Pedestal;
	
	
	struct CaptionParameters
	{
		std::string  itsText;
		bool         itIsWrapped;
		
		CaptionParameters() : itIsWrapped( true )
		{
		}
	};
	
	typedef std::map< const FSTree*, CaptionParameters > CaptionParametersMap;
	
	static CaptionParametersMap gCaptionParametersMap;
	
	
	class Caption : public Ped::Caption
	{
		private:
			typedef const FSTree* Key;
			
			Key itsKey;
		
		public:
			Caption( Key key ) : itsKey( key )
			{
			}
			
			std::string Text() const;
			
			bool Wrapped() const;
	};
	
	std::string Caption::Text() const
	{
		std::string result;
		
		CaptionParametersMap::const_iterator it = gCaptionParametersMap.find( itsKey );
		
		if ( it != gCaptionParametersMap.end() )
		{
			result = it->second.itsText;
		}
		
		std::replace( result.begin(), result.end(), '\n', '\r' );
		
		return result;
	}
	
	bool Caption::Wrapped() const
	{
		CaptionParametersMap::const_iterator it = gCaptionParametersMap.find( itsKey );
		
		if ( it != gCaptionParametersMap.end() )
		{
			return it->second.itIsWrapped;
		}
		
		return true;
	}
	
	boost::shared_ptr< Ped::View > CaptionFactory( const FSTree* delegate )
	{
		return boost::shared_ptr< Ped::View >( new Caption( delegate ) );
	}
	
	
	void FSTree_new_caption::DestroyDelegate( const FSTree* delegate )
	{
		gCaptionParametersMap.erase( delegate );
	}
	
	
	static void CaptionText_SetEOF( const FSTree* text, off_t length )
	{
		const FSTree* view = text->ParentRef().get();
		
		gCaptionParametersMap[ view ].itsText.resize( length );
		
		InvalidateWindowForView( view );
	}
	
	class CaptionTextFileHandle : public VirtualFileHandle
	{
		public:
			CaptionTextFileHandle( const FSTreePtr& file, OpenFlags flags ) : VirtualFileHandle( file, flags )
			{
			}
			
			boost::shared_ptr< IOHandle > Clone();
			
			const FSTree* ViewKey() const;
			
			std::string& String() const  { return gCaptionParametersMap[ ViewKey() ].itsText; }
			
			ssize_t SysRead( char* buffer, std::size_t byteCount );
			
			ssize_t SysWrite( const char* buffer, std::size_t byteCount );
			
			off_t GetEOF() const  { return String().size(); }
			
			void SetEOF( off_t length )  { CaptionText_SetEOF( GetFile().get(), length ); }
	};
	
	boost::shared_ptr< IOHandle > CaptionTextFileHandle::Clone()
	{
		return boost::shared_ptr< IOHandle >( new CaptionTextFileHandle( GetFile(), GetFlags() ) );
	}
	
	const FSTree* CaptionTextFileHandle::ViewKey() const
	{
		return GetFile()->ParentRef().get();
	}
	
	ssize_t CaptionTextFileHandle::SysRead( char* buffer, std::size_t byteCount )
	{
		std::string& s = String();
		
		ASSERT( GetFileMark() <= s.size() );
		
		byteCount = std::min( byteCount, s.size() - GetFileMark() );
		
		std::copy( s.begin() + GetFileMark(),
		           s.begin() + GetFileMark() + byteCount,
		           buffer );
		
		return Advance( byteCount );
	}
	
	ssize_t CaptionTextFileHandle::SysWrite( const char* buffer, std::size_t byteCount )
	{
		std::string& s = String();
		
		if ( GetFileMark() + byteCount > s.size() )
		{
			s.resize( GetFileMark() + byteCount );
		}
		
		std::copy( buffer,
		           buffer + byteCount,
		           s.begin() + GetFileMark() );
		
		const FSTree* view = ViewKey();
		
		InvalidateWindowForView( view );
		
		return Advance( byteCount );
	}
	
	class FSTree_Caption_text : public FSTree
	{
		public:
			FSTree_Caption_text( const FSTreePtr&    parent,
			                     const std::string&  name ) : FSTree( parent, name )
			{
			}
			
			std::string& String() const  { return gCaptionParametersMap[ ParentRef().get() ].itsText; }
			
			mode_t FilePermMode() const  { return S_IRUSR | S_IWUSR; }
			
			off_t GetEOF() const  { return String().size(); }
			
			void SetEOF( off_t length ) const  { CaptionText_SetEOF( this, length ); }
			
			boost::shared_ptr< IOHandle > Open( OpenFlags flags ) const;
	};
	
	boost::shared_ptr< IOHandle > FSTree_Caption_text::Open( OpenFlags flags ) const
	{
		IOHandle* result = new CaptionTextFileHandle( Self(), flags );
		
		return boost::shared_ptr< IOHandle >( result );
	}
	
	
	namespace
	{
		
		bool& Wrapped( const FSTree* view )
		{
			return gCaptionParametersMap[ view ].itIsWrapped;
		}
		
	}
	
	template < class Property >
	static FSTreePtr Property_Factory( const FSTreePtr&    parent,
	                                   const std::string&  name )
	{
		return FSTreePtr( new FSTree_Property( parent,
		                                       name,
		                                       &Property::Get,
		                                       &Property::Set ) );
	}
	
	const FSTree_Premapped::Mapping Caption_view_Mappings[] =
	{
		{ "text", &Basic_Factory< FSTree_Caption_text > },
		
		{ "wrapped", &Property_Factory< View_Property< Boolean_Scribe, Wrapped > > },
		
		{ NULL, NULL }
	};
	
}

