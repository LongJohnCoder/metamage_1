/*	======================
 *	FSTree_new_textedit.cc
 *	======================
 */

#include "Genie/FS/FSTree_new_textedit.hh"

// plus
#include "plus/serialize.hh"

// Genie
#include "Genie/FS/FSTree_Directory.hh"
#include "Genie/FS/FSTree_Property.hh"
#include "Genie/FS/sys/port/ADDR.hh"
#include "Genie/FS/TextEdit.hh"
#include "Genie/FS/TextEdit_text.hh"
#include "Genie/FS/Trigger.hh"
#include "Genie/FS/Views.hh"
#include "Genie/IO/Stream.hh"
#include "Genie/IO/VirtualFile.hh"


namespace Genie
{
	
	namespace Ped = Pedestal;
	
	
	static boost::intrusive_ptr< Ped::View > CreateView( const FSTree* delegate )
	{
		return seize_ptr( new TextEdit_Scroller( delegate ) );
	}
	
	
	static void DestroyDelegate( const FSTree* delegate )
	{
		ScrollerParameters::Erase( delegate );
		
		TextEditParameters::Erase( delegate );
	}
	
	
	class FSTree_TextEdit_interlock : public FSTree
	{
		public:
			FSTree_TextEdit_interlock( const FSTreePtr&     parent,
			                           const plus::string&  name )
			:
				FSTree( parent, name, S_IFREG | 0600 )
			{
			}
			
			void SetTimes() const;
	};
	
	void FSTree_TextEdit_interlock::SetTimes() const
	{
		const FSTree* view = ParentRef().get();
		
		TextEditParameters::Get( view ).itIsInterlocked = true;
	}
	
	
	class TextEdit_gate_Handle : public VirtualFileHandle< StreamHandle >
	{
		public:
			TextEdit_gate_Handle( const FSTreePtr& file, OpenFlags flags )
			:
				VirtualFileHandle< StreamHandle >( file, flags )
			{
			}
			
			unsigned SysPoll();
			
			ssize_t SysRead( char* buffer, size_t n_bytes );
	};
	
	unsigned TextEdit_gate_Handle::SysPoll()
	{
		const FSTree* view = GetFile()->ParentRef().get();
		
		TextEditParameters& params = TextEditParameters::Get( view );
		
		const bool readable = !params.itIsInterlocked;
		
		return readable * kPollRead | kPollWrite;
	}
	
	ssize_t TextEdit_gate_Handle::SysRead( char* buffer, size_t n_bytes )
	{
		const FSTree* view = GetFile()->ParentRef().get();
		
		TextEditParameters& params = TextEditParameters::Get( view );
		
		while ( params.itIsInterlocked )
		{
			TryAgainLater();
		}
		
		return 0;
	}
	
	
	class FSTree_TextEdit_gate : public Trigger_Base
	{
		public:
			FSTree_TextEdit_gate( const FSTreePtr&     parent,
			                      const plus::string&  name )
			:
				Trigger_Base( parent, name )
			{
			}
			
			void Invoke() const;
			
			mode_t FilePermMode() const  { return S_IRUSR | S_IWUSR; }
			
			boost::shared_ptr< IOHandle > Open( OpenFlags flags ) const;
	};
	
	void FSTree_TextEdit_gate::Invoke() const
	{
		const FSTree* view = ParentRef().get();
		
		TextEditParameters& params = TextEditParameters::Get( view );
		
		params.itIsInterlocked = false;
	}
	
	boost::shared_ptr< IOHandle > FSTree_TextEdit_gate::Open( OpenFlags flags ) const
	{
		if ( (flags & O_ACCMODE) == O_WRONLY )
		{
			return Trigger_Base::Open( flags );
		}
		
		return seize_ptr( new TextEdit_gate_Handle( Self(), flags ) );
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
	
	
	#define PROPERTY( prop )  &new_property, &property_params_factory< prop >::value
	
	typedef Const_View_Property< plus::serialize_bool, TextEditParameters::Active >  Active_Property;
	
	typedef TextInvalidating_View_Property< plus::serialize_bool, TextEditParameters::Secret >  Secret_Property;
	
	typedef View_Property< plus::serialize_bool, TextEditParameters::Singular >  Singular_Property;
	typedef View_Property< plus::serialize_bool, TextEditParameters::Wrapped  >  Wrapped_Property;
	
	typedef View_Property< plus::serialize_int< int >, ScrollerParameters::Width  >  Width_Property;
	typedef View_Property< plus::serialize_int< int >, ScrollerParameters::Height >  Height_Property;
	
	typedef TE_View_Property< plus::serialize_int< int >, ScrollerParameters::HOffset >  HOffset_Property;
	typedef TE_View_Property< plus::serialize_int< int >, ScrollerParameters::VOffset >  VOffset_Property;
	
	static const FSTree_Premapped::Mapping local_mappings[] =
	{
		{ "text", &New_FSTree_TextEdit_text },
		
		{ "gate", &Basic_Factory< FSTree_TextEdit_gate > },
		
		{ "interlock", &Basic_Factory< FSTree_TextEdit_interlock > },
		
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
	
	FSTreePtr New_FSTree_new_textedit( const FSTreePtr&     parent,
	                                   const plus::string&  name,
	                                   const void*          args )
	{
		return seize_ptr( new FSTree_new_View( parent,
		                                       name,
		                                       &CreateView,
		                                       local_mappings,
		                                       &DestroyDelegate ) );
	}
	
}

