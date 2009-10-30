/*	===================
 *	iota/hexidecimal.hh
 *	===================
 */

#ifndef IOTA_HEXIDECIMAL_HH
#define IOTA_HEXIDECIMAL_HH


namespace iota
{
	
	// (hex-digit & 0x1f) -> numeric nibble
	extern unsigned char decoded_hex_table[];
	
	// (nibble & 0x0f) -> ASCII hex digit
	extern char encoded_hex_table[];
	
	
	inline unsigned char decoded_hex_digit( char c )
	{
		return decoded_hex_table[ c & 0x1f ];
	}
	
	inline char encoded_hex_char( unsigned char d )
	{
		return encoded_hex_table[ d & 0x0f ];
	}
	
	
	inline unsigned short hexidecimal_magnitude( unsigned x )
	{
		const unsigned short result = x >= 0x10000000 ? 8
		                            : x >= 0x01000000 ? 7
		                            : x >= 0x00100000 ? 6
		                            : x >= 0x00010000 ? 5
		                            : x >= 0x00001000 ? 4
		                            : x >= 0x00000100 ? 3
		                            : x >= 0x00000010 ? 2
		                            :                   1;
		
		return result;
	}
	
	inline char printable_hex_digit( unsigned short x )
	{
		x &= 0xf;
		
		return x + (x < 10 ? '0'
		                   : 'a' - 10);
	}
	
	void inscribe_n_hex_digits( char* p, unsigned long x, unsigned short n );
	
}

#endif

