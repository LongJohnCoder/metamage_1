/*
	text_input/feed.cc
	------------------
*/

#include "text_input/feed.hh"

// Standard C
#include <string.h>

// debug
#include "debug/assert.hh"


namespace text_input
{
	
	static inline bool is_complete_line( const plus::string& s )
	{
		return !s.empty()  &&  *(s.end() - 1) == '\n';
	}
	
	
	void feed::advance_CRLF()
	{
		if ( its_last_end_was_CR  &&  its_mark < its_data_length  &&  its_buffer[ its_mark ] == '\n' )
		{
			++its_mark;
			
			its_last_end_was_CR = false;
		}
	}
	
	void feed::prime()
	{
		if ( it_was_returned )
		{
			its_next_line.resize( 0 );
			
			it_was_returned = false;
		}
		
		if ( is_complete_line( its_next_line ) )
		{
			return;
		}
		
		const char* begin = &its_buffer[ its_mark        ];
		const char* end   = &its_buffer[ its_data_length ];
		
		const char* p = begin;
		
		while ( (p < end)  &&  (*p != '\r')  &&  (*p != '\n') )
		{
			++p;
		}
		
		its_next_line.append( begin, p );
		
		if ( p != end )
		{
			its_next_line.push_back( '\n' );
			
			its_last_end_was_CR = *p++ == '\r';
		}
		
		its_mark = p - its_buffer;
		
		advance_CRLF();
	}
	
	const plus::string* feed::get_line()
	{
		prime();
		
		const bool complete_line = is_complete_line( its_next_line );
		
		it_was_returned = complete_line;
		
		return complete_line ? &its_next_line : NULL;
	}
	
	const plus::string& feed::get_fragment_ref()
	{
		prime();
		
		it_was_returned = true;
		
		return its_next_line;
	}
	
	const plus::string* feed::get_fragment()
	{
		(void) get_fragment_ref();
		
		return its_next_line.empty() ? NULL : &its_next_line;
	}
	
	char* feed::buffer()
	{
		if ( its_mark != its_data_length )
		{
			ASSERT( its_mark < its_data_length );
			
			throw buffer_occupied();
		}
		
		return its_buffer;
	}
	
	void feed::accept_input( size_type length )
	{
		ASSERT( length <= buffer_length );
		
		ASSERT( its_mark == its_data_length );
		
		its_data_length = length;
		its_mark        = 0;
		
		advance_CRLF();
	}
	
	void feed::accept_input( const char* buffer, size_type length )
	{
		if ( length > buffer_length )
		{
			throw buffer_overrun();
		}
		
		memcpy( this->buffer(), buffer, length );
		
		accept_input( length );
	}
	
}

