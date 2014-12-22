/*
	memory.cc
	---------
*/

#include "v68k-mac/memory.hh"

// Standard C
#include <string.h>

// Standard C++
#include <algorithm>

// v68k
#include "v68k/endian.hh"

// v68k-mac
#include "v68k-mac/dynamic_globals.hh"


#pragma exceptions off


namespace v68k {
namespace mac  {

enum
{
	tag_Ticks,
	tag_Ticks_low_word,
	tag_MemErr,
	tag_ScrnBase,
	tag_ScrnBase_low_word,
	tag_WMgrPort,
	tag_WMgrPort_low_word,
	tag_last_A_trap,
	n_words
};

static uint16_t words[ n_words ];

struct global
{
	uint16_t  addr;
	uint8_t   size_;
	uint8_t   index;
	
	uint8_t size() const  { return size_ & 0x3F; }
	
	uint16_t word() const  { return int16_t( int8_t( index ) ); }
};

static inline bool operator<( const global& g, uint16_t addr )
{
	return g.addr + g.size() <= addr;
}

static const global globals[] =
{
	{ 0x0102, 0x84, 72              },  // ScrVRes, ScrHRes
	{ 0x0106, 0x82, 64              },  // ScreenRow
	{ 0x016A, 0x44, tag_Ticks       },
	{ 0x0220, 2,    tag_MemErr      },
	{ 0x031A, 0x83, 0xFF            },  // Lo3Bytes
	{ 0x0824, 4,    tag_ScrnBase    },
	{ 0x09DE, 4,    tag_WMgrPort    },
	{ 0x0A02, 0x84, 0x01            },  // OneOne
	{ 0x0A06, 0x84, 0xFF            },  // MinusOne
	{ 0x0BFE, 2,    tag_last_A_trap }
};

static const global* find_global( uint16_t address )
{
	const global* begin = globals;
	const global* end   = globals + sizeof globals / sizeof globals[0];
	
	const global* it = std::lower_bound( begin, end, address );
	
	if ( it == end )
	{
		return NULL;
	}
	
	if ( address < it->addr )
	{
		return NULL;
	}
	
	return it;
}

static void refresh_dynamic_global( uint8_t tag )
{
	uint16_t* address = &words[ tag ];
	
	uint32_t longword;
	
	switch ( tag )
	{
		case tag_Ticks:
			longword = get_Ticks();
			
			*(uint32_t*) address = big_longword( longword );
			
			break;
		
		default:
			break;
	}
}

static uint8_t buffer[ 7 ];

static uint8_t* read_globals( const global* g, uint32_t addr, uint32_t size )
{
	// size == 1 -> offset = 0
	// size == 2 -> offset = addr & 1
	// size == 4 -> offset = addr & 3
	
	const int offset = addr & size - 1;
	
	int i = offset - (addr - g->addr);
	
	const uint32_t end = addr + size;
	
	for ( ;; )
	{
		uint32_t width = g->size();
		
		if ( int8_t( g->size_ ) < 0 )
		{
			if ( width < 2 )
			{
				buffer[ i++ ] = g->index;
			}
			else
			{
				const uint16_t word = g->word();
				
				buffer[ i++ ] = word >> 8;
				buffer[ i++ ] = word;
				
				if ( width > 2 )
				{
					buffer[ i++ ] = word >> 8;
					buffer[ i++ ] = word;
					
					if ( width == 3 )
					{
						buffer[ i - 4 ] = 0;
						
						width = 4;
					}
				}
			}
		}
		else
		{
			if ( g->size_ >= 0x40 )
			{
				refresh_dynamic_global( g->index );
			}
			
			return (uint8_t*) &words[ g->index ];
		}
		
		addr += width;
		
		if ( addr >= end )
		{
			return buffer + offset;
		}
		
		if ( ++g == (globals + sizeof globals / sizeof globals[0]) )
		{
			return NULL;
		}
		
		if ( g->addr != addr )
		{
			return NULL;
		}
	}
	
	return buffer + offset;
}

static uint8_t* write_globals( const global* g, uint32_t addr, uint32_t size )
{
	if ( g->addr == addr  &&  g->size_ == size )
	{
		return buffer;
	}
	
	return NULL;
}

static uint8_t* update_globals( const global* g, uint32_t addr, uint32_t size )
{
	memcpy( &words[ g->index ], buffer, size );
	
	return buffer;
}

uint8_t* low_memory::translate( uint32_t               addr,
                                uint32_t               length,
                                v68k::function_code_t  fc,
                                v68k::memory_access_t  access ) const
{
	if ( access == mem_exec )
	{
		// None of Mac low memory is executable
		
		return 0;  // NULL
	}
	
	if ( const global* g = find_global( addr ) )
	{
		if ( access == mem_read )
		{
			return read_globals( g, addr, length );
		}
		else if ( access == mem_write )
		{
			return write_globals( g, addr, length );
		}
		else  // mem_update
		{
			return update_globals( g, addr, length );
		}
	}
	
	return 0;
}

}  // namespace mac
}  // namespace v68k

