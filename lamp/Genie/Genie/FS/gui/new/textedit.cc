/*
	gui/new/textedit.cc
	-------------------
*/

#include "Genie/FS/gui/new/textedit.hh"

// POSIX
#include <sys/stat.h>

// plus
#include "plus/serialize.hh"

// vfs
#include "vfs/filehandle.hh"
#include "vfs/enum/poll_result.hh"
#include "vfs/filehandle/functions/nonblocking.hh"
#include "vfs/filehandle/methods/filehandle_method_set.hh"
#include "vfs/filehandle/methods/stream_method_set.hh"
#include "vfs/filehandle/primitives/get_file.hh"
#include "vfs/methods/data_method_set.hh"
#include "vfs/methods/node_method_set.hh"
#include "vfs/node/types/fixed_dir.hh"

// Genie
#include "Genie/api/yield.hh"
#include "Genie/FS/FSTree.hh"
#include "Genie/FS/FSTree_Property.hh"
#include "Genie/FS/TextEdit.hh"
#include "Genie/FS/TextEdit_text.hh"
#include "Genie/FS/Trigger.hh"
#include "Genie/FS/Views.hh"


namespace Genie
{
	
	namespace Ped = Pedestal;
	
	
	static boost::intrusive_ptr< Ped::View > CreateView( const FSTree* delegate )
	{
		return new TextEdit_Scroller( delegate );
	}
	
	
	static void DestroyDelegate( const FSTree* delegate )
	{
		ScrollerParameters::Erase( delegate );
		
		TextEditParameters::Erase( delegate );
	}
	
	
	static void textedit_lock_trigger( const FSTree* that )
	{
		const FSTree* view = that->owner();
		
		const bool locked = that->name()[0] != 'u';
		
		TextEditParameters::Get( view ).itIsInterlocked = locked;
	}
	
	
	class TextEdit_gate_Handle : public vfs::filehandle
	{
		public:
			TextEdit_gate_Handle( const vfs::node& file, int flags );
			
			unsigned SysPoll();
			
			ssize_t SysRead( char* buffer, size_t n_bytes );
	};
	
	
	static unsigned texteditgate_poll( vfs::filehandle* that )
	{
		return static_cast< TextEdit_gate_Handle& >( *that ).SysPoll();
	}
	
	static ssize_t texteditgate_read( vfs::filehandle* that, char* buffer, size_t n )
	{
		return static_cast< TextEdit_gate_Handle& >( *that ).SysRead( buffer, n );
	}
	
	static const vfs::stream_method_set texteditgate_stream_methods =
	{
		&texteditgate_poll,
		&texteditgate_read,
	};
	
	static const vfs::filehandle_method_set texteditgate_methods =
	{
		NULL,
		NULL,
		&texteditgate_stream_methods,
	};
	
	
	TextEdit_gate_Handle::TextEdit_gate_Handle( const vfs::node& file, int flags )
	:
		vfs::filehandle( &file, flags, &texteditgate_methods )
	{
	}
	
	unsigned TextEdit_gate_Handle::SysPoll()
	{
		const FSTree* view = get_file( *this )->owner();
		
		TextEditParameters& params = TextEditParameters::Get( view );
		
		const bool readable = !params.itIsInterlocked;
		
		return readable * vfs::Poll_read | vfs::Poll_write;
	}
	
	ssize_t TextEdit_gate_Handle::SysRead( char* buffer, size_t n_bytes )
	{
		const FSTree* view = get_file( *this )->owner();
		
		TextEditParameters& params = TextEditParameters::Get( view );
		
		while ( params.itIsInterlocked )
		{
			try_again( is_nonblocking( *this ) );
		}
		
		return 0;
	}
	
	
	static vfs::filehandle_ptr textedit_gate_open( const FSTree* that, int flags, mode_t mode )
	{
		return new TextEdit_gate_Handle( *that, flags );
	}
	
	static const vfs::data_method_set textedit_gate_data_methods =
	{
		&textedit_gate_open
	};
	
	static const vfs::node_method_set textedit_gate_methods =
	{
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		&textedit_gate_data_methods
	};
	
	
	static FSTreePtr gate_factory( const FSTree*        parent,
	                               const plus::string&  name,
	                               const void*          args )
	{
		return new FSTree( parent, name, S_IFREG | 0400, &textedit_gate_methods );
	}
	
	template < class Serialize, typename Serialize::result_type& (*Access)( const FSTree* ) >
	struct TE_View_Property : public View_Property< Serialize, Access >
	{
		static void Set( const FSTree* that, const char* begin, const char* end, bool binary )
		{
			TextEditParameters::Get( that ).itHasChangedAttributes = true;
			
			View_Property< Serialize, Access >::Set( that, begin, end, binary );
		}
	};
	
	template < class Serialize, typename Serialize::result_type& (*Access)( const FSTree* ) >
	struct TextInvalidating_View_Property : public View_Property< Serialize, Access >
	{
		static void Set( const FSTree* that, const char* begin, const char* end, bool binary )
		{
			TextEditParameters::Get( that ).itsValidLength = 0;
			
			View_Property< Serialize, Access >::Set( that, begin, end, binary );
		}
	};
	
	static const trigger_extra textedit_lock_trigger_extra =
	{
		&textedit_lock_trigger
	};
	
	
	#define PROPERTY( prop )  &new_property, &property_params_factory< prop >::value
	
	typedef Const_View_Property< plus::serialize_bool, TextEditParameters::Active >  Active_Property;
	
	typedef TextInvalidating_View_Property< plus::serialize_bool, TextEditParameters::Secret >  Secret_Property;
	
	typedef View_Property< plus::serialize_bool, TextEditParameters::Singular >  Singular_Property;
	typedef View_Property< plus::serialize_bool, TextEditParameters::Wrapped  >  Wrapped_Property;
	
	typedef View_Property< plus::serialize_int< int >, ScrollerParameters::Width  >  Width_Property;
	typedef View_Property< plus::serialize_int< int >, ScrollerParameters::Height >  Height_Property;
	
	typedef TE_View_Property< plus::serialize_int< int >, ScrollerParameters::HOffset >  HOffset_Property;
	typedef TE_View_Property< plus::serialize_int< int >, ScrollerParameters::VOffset >  VOffset_Property;
	
	static const vfs::fixed_mapping local_mappings[] =
	{
		{ "text", &New_FSTree_TextEdit_text },
		
		{ "lock",   &trigger_factory, &textedit_lock_trigger_extra },
		{ "unlock", &trigger_factory, &textedit_lock_trigger_extra },
		
		{ "gate", &gate_factory },
		
		{ "selection", PROPERTY( Selection_Property ) },
		
		{ "active", PROPERTY( Active_Property ) },
		
		{ "secret", PROPERTY( Secret_Property ) },
		
		{ "singular", PROPERTY( Singular_Property ) },
		
		//{ "wrapped", PROPERTY( Wrapped_Property ) },
		
		// unlocked-text
		
		{ "width",  PROPERTY( Width_Property  ) },
		{ "height", PROPERTY( Height_Property ) },
		
		{ "x", PROPERTY( HOffset_Property ) },
		{ "y", PROPERTY( VOffset_Property ) },
		
		{ NULL, NULL }
	};
	
	FSTreePtr New_FSTree_new_textedit( const FSTree*        parent,
	                                   const plus::string&  name,
	                                   const void*          args )
	{
		return New_new_view( parent,
		                     name,
		                     &CreateView,
		                     local_mappings,
		                     &DestroyDelegate );
	}
	
}

