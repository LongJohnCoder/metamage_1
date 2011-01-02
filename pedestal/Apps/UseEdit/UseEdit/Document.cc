/*	===========
 *	Document.cc
 *	===========
 */

#include "UseEdit/Document.hh"

// Standard C++
#include <algorithm>

// iota
#include "iota/convert_string.hh"

// plus
#include "plus/var_string.hh"

// nucleus
#include "nucleus/scribe.hh"

// Io
#include "io/slurp.hh"

// Nitrogen
#include "Nitrogen/MacWindows.hh"

// MacFiles
#include "MacFiles/Classic.hh"
#include "MacFiles/Unicode.hh"


namespace UseEdit
{
	
	namespace n = nucleus;
	namespace N = Nitrogen;
	namespace Ped = Pedestal;
	
	template < class FileSpec >
	static plus::string ReadFileData( const FileSpec& file )
	{
		plus::var_string data = io::slurp_file< n::POD_vector_scribe< plus::var_string > >( file );
		
		// Allow LF newlines
		std::replace( data.begin(),
		              data.end(),
		              '\n',
		              '\r' );
		
		return data;
	}
	
	static n::owned< CFStringRef > GetFilenameAsCFString( const FSRef& file )
	{
		N::FSGetCatalogInfo_Result info = N::FSGetCatalogInfo( file, kFSCatInfoNone );
		
		return n::convert< n::owned< CFStringRef > >( info.outName );
	}
	
	
	Document::Document()
	: 
		itsWindow( NewWindow() ),
		itHasFile( false ),
		itIsDirty( false )   // A new document is never dirty, even if not saved
	{
	}
	
	static void LoadText( Ped::Window& window, const plus::string& text )
	{
		View& scrollframe = reinterpret_cast< View& >( *window.GetView() );
		
		SetText( scrollframe, text );
	}
	
	Document::Document( const FSSpec& file )
	: 
		itsWindow( NewWindow( file.name ) ),
		itHasFile( true  ),
		itIsDirty( false )
	{
		LoadText( *itsWindow, ReadFileData( file ) );
	}
	
	Document::Document( const FSRef& file )
	: 
		itsWindow( NewWindow() ),
		itHasFile( true  ),
		itIsDirty( false )
	{
		N::SetWindowTitleWithCFString( N::FrontWindow(),
		                               GetFilenameAsCFString( file ) );
		
		LoadText( *itsWindow, ReadFileData( file ) );
	}
	
	plus::string Document::GetName() const
	{
		return iota::convert_string< plus::string >( N::GetWTitle( GetWindowRef() ) );
	}
	
}

