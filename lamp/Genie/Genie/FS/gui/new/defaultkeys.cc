/*
	gui/new/defaultkeys.cc
	----------------------
*/

#include "Genie/FS/gui/new/defaultkeys.hh"

// Mac OS
#ifndef __EVENTS__
#include <Events.h>
#endif

// Pedestal
#include "Pedestal/View.hh"

// Genie
#include "Genie/FS/FSTree_Directory.hh"
#include "Genie/FS/Views.hh"


namespace Genie
{
	
	namespace Ped = Pedestal;
	
	
	class DefaultKey_Handler : public Ped::View
	{
		private:
			const FSTree* itsKey;
		
		public:
			DefaultKey_Handler( const FSTree* key ) : itsKey( key )
			{
			}
			
			bool KeyDown( const EventRecord& event );
	};
	
	static void Update( const FSTree* that, const char* name )
	{
		const FSTree* window = GetViewWindowKey( that );
		
		ASSERT( window != NULL );
		
		FSTreePtr link = window->Lookup( name );
		
		FSTreePtr target = link->ResolveLink();
		
		target->SetTimes();
	}
	
	bool DefaultKey_Handler::KeyDown( const EventRecord& event )
	{
		if ( event.modifiers & (controlKey | rightControlKey) )
		{
			return false;
		}
		
		const char c = event.message & charCodeMask;
		
		switch ( c )
		{
			case kEnterCharCode:
			case kReturnCharCode:
				Update( itsKey, "accept" );
				
				return true;
			
			case kEscapeCharCode:
				switch ( const char keyCode = (event.message & keyCodeMask) >> 8 )
				{
					// Escape
					case 0x35:
						Update( itsKey, "cancel" );
						
						return true;
					
					// Clear
					case 0x47:
					default:
						break;
				}
				break;
			
			default:
				break;
		}
		
		return false;
	}
	
	static boost::intrusive_ptr< Ped::View > CreateView( const FSTree* delegate )
	{
		return new DefaultKey_Handler( delegate );
	}
	
	
	static void DestroyDelegate( const FSTree* delegate )
	{
	}
	
	
	static const premapped::mapping local_mappings[] =
	{
		{ NULL, NULL }
	};
	
	FSTreePtr New_defaultkeys( const FSTreePtr&     parent,
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

