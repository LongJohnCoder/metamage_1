/*
	serialize_Str255.hh
	-------------------
*/

#ifndef GENIE_FS_SERIALIZESTR255_HH
#define GENIE_FS_SERIALIZESTR255_HH

// plus
#include "plus/stringify.hh"

// Nitrogen
#ifndef NITROGEN_STR_HH
#include "Nitrogen/Str.hh"
#endif


namespace Genie
{
	
	struct vivify_Str255
	{
		typedef Nitrogen::Str255 result_type;
		
		static Nitrogen::Str255 apply( const char* begin, const char* end )
		{
			return Nitrogen::Str255( begin, end - begin );
		}
	};
	
	struct deconstruct_Str255_contents
	{
		typedef plus::stringify_pascal_string  stringify;
		typedef plus::stringify_pascal_string  freeze;
		
		static void apply( plus::var_string& out, const unsigned char* s, bool )
		{
			stringify::apply( out, s );
		}
	};
	
	struct reconstruct_Str255_contents
	{
		typedef Nitrogen::Str255 result_type;
		
		typedef vivify_Str255  vivify;
		typedef vivify_Str255  thaw;
		
		static Nitrogen::Str255 apply( const char* begin, const char* end, bool )
		{
			return vivify::apply( begin, end );
		}
	};
	
	struct serialize_Str255_contents
	{
		typedef Nitrogen::Str255 result_type;
		
		static const int fixed_size = -1;
		
		typedef deconstruct_Str255_contents  deconstruct;
		typedef reconstruct_Str255_contents  reconstruct;
	};
	
}

#endif

