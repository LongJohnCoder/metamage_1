/*	================
 *	TextEdit_text.cc
 *	================
 */

#include "Genie/FS/TextEdit_text.hh"

// POSIX
#include <sys/stat.h>

// plus
#include "plus/mac_utf8.hh"

// vfs
#include "vfs/filehandle.hh"
#include "vfs/filehandle/methods/bstore_method_set.hh"
#include "vfs/filehandle/methods/filehandle_method_set.hh"
#include "vfs/filehandle/primitives/get_file.hh"
#include "vfs/methods/data_method_set.hh"
#include "vfs/methods/node_method_set.hh"

// Genie
#include "Genie/FS/FSTree.hh"
#include "Genie/FS/TextEdit.hh"
#include "Genie/FS/Views.hh"


namespace Genie
{
	
	namespace Ped = Pedestal;
	
	
	template < class T >
	static inline T min( T a, T b )
	{
		return b < a ? b : a;
	}
	
	
	static void TextEdit_text_SetEOF( const FSTree* text, off_t length )
	{
		const FSTree* view = text->owner();
		
		TextEditParameters& params = TextEditParameters::Get( view );
		
		if ( length < params.its_utf8_text.length() )
		{
			params.its_utf8_text.resize( length );
			
			params.its_mac_text = plus::mac_from_utf8( params.its_utf8_text );
			
			params.itsValidLength = params.its_mac_text.length();
		}
		
		InvalidateWindowForView( view );
	}
	
	class TextEdit_text_Handle : public vfs::filehandle
	{
		public:
			TextEdit_text_Handle( const vfs::node& file, int flags );
			
			const FSTree* ViewKey();
			
			ssize_t Positioned_Read( char* buffer, size_t n_bytes, off_t offset );
			
			ssize_t Positioned_Write( const char* buffer, size_t n_bytes, off_t offset );
			
			off_t GetEOF()  { return TextEditParameters::Get( ViewKey() ).its_utf8_text.size(); }
			
			void SetEOF( off_t length )  { TextEdit_text_SetEOF( get_file( *this ).get(), length ); }
	};
	
	
	static ssize_t TextEdit_text_pread( vfs::filehandle* file, char* buffer, size_t n, off_t offset )
	{
		return static_cast< TextEdit_text_Handle& >( *file ).Positioned_Read( buffer, n, offset );
	}
	
	static off_t TextEdit_text_geteof( vfs::filehandle* file )
	{
		return static_cast< TextEdit_text_Handle& >( *file ).GetEOF();
	}
	
	static ssize_t TextEdit_text_pwrite( vfs::filehandle* file, const char* buffer, size_t n, off_t offset )
	{
		return static_cast< TextEdit_text_Handle& >( *file ).Positioned_Write( buffer, n, offset );
	}
	
	static void TextEdit_text_seteof( vfs::filehandle* file, off_t length )
	{
		static_cast< TextEdit_text_Handle& >( *file ).SetEOF( length );
	}
	
	static const vfs::bstore_method_set TextEdit_text_bstore_methods =
	{
		&TextEdit_text_pread,
		&TextEdit_text_geteof,
		&TextEdit_text_pwrite,
		&TextEdit_text_seteof,
	};
	
	static const vfs::filehandle_method_set TextEdit_text_methods =
	{
		&TextEdit_text_bstore_methods,
	};
	
	
	TextEdit_text_Handle::TextEdit_text_Handle( const vfs::node& file, int flags )
	:
		vfs::filehandle( &file, flags, &TextEdit_text_methods )
	{
	}
	
	const FSTree* TextEdit_text_Handle::ViewKey()
	{
		return get_file( *this )->owner();
	}
	
	ssize_t TextEdit_text_Handle::Positioned_Read( char* buffer, size_t n_bytes, off_t offset )
	{
		const FSTree* view = ViewKey();
		
		TextEditParameters& params = TextEditParameters::Get( view );
		
		const plus::string& s = params.its_utf8_text;
		
		if ( offset >= s.size() )
		{
			return 0;
		}
		
		n_bytes = min< size_t >( n_bytes, s.size() - offset );
		
		memcpy( buffer, &s[ offset ], n_bytes );
		
		return n_bytes;
	}
	
	ssize_t TextEdit_text_Handle::Positioned_Write( const char* buffer, size_t n_bytes, off_t offset )
	{
		const FSTree* view = ViewKey();
		
		TextEditParameters& params = TextEditParameters::Get( view );
		
		plus::var_string& s = params.its_utf8_text;
		
		if ( offset + n_bytes > s.size() )
		{
			s.resize( offset + n_bytes );
		}
		
		memcpy( s.begin() + offset, buffer, n_bytes );
		
		if ( offset < params.itsValidLength )
		{
			params.itsValidLength = 0;  // temporary hack until we can count UTF-8 chars
		}
		
		params.its_mac_text = plus::mac_from_utf8( s );
		
		InvalidateWindowForView( view );
		
		return n_bytes;
	}
	
	
	static off_t textedit_text_geteof( const FSTree* that )
	{
		return TextEditParameters::Get( that->owner() ).its_utf8_text.size();
	}
	
	static void textedit_text_seteof( const FSTree* that, off_t length )
	{
		TextEdit_text_SetEOF( that, length );
	}
	
	static vfs::filehandle_ptr textedit_text_open( const FSTree* that, int flags, mode_t mode )
	{
		return new TextEdit_text_Handle( *that, flags );
	}
	
	static const vfs::data_method_set textedit_text_data_methods =
	{
		&textedit_text_open,
		&textedit_text_geteof,
		&textedit_text_seteof
	};
	
	static const vfs::node_method_set textedit_text_methods =
	{
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		&textedit_text_data_methods
	};
	
	FSTreePtr New_FSTree_TextEdit_text( const FSTree*        parent,
	                                    const plus::string&  name,
	                                    const void*          args )
	{
		return new FSTree( parent, name, S_IFREG | 0600, &textedit_text_methods );
	}
	
}

