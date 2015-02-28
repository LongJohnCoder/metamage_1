/*
	utf8_resinfo_name.cc
	--------------------
*/

#include "rsrc/utf8_resinfo_name.hh"

// plus
#include "plus/mac_utf8.hh"

// MacScribe
#include "rsrc/resinfo_name.hh"



namespace MacScribe
{
	
	plus::string get_utf8_name_from_ResInfo( const mac::types::ResInfo& info )
	{
		return plus::utf8_from_mac( get_name_from_ResInfo( info ) );
	}
	
}
