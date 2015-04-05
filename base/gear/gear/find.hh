/*
	gear/find.hh
	------------
*/

#ifndef GEAR_FIND_HH
#define GEAR_FIND_HH


namespace gear
{
	
	const char* find_first_match( const char*  p,
	                              const char*  end,
	                              char         c,
	                              const char*  _default = 0,
	                              bool         negated  = false );
	
	const char* find_last_match( const char*  p,
	                             const char*  end,
	                             char         c,
	                             const char*  _default = 0,
	                             bool         negated  = false );
	
	const char* find_first_match( const char*           p,
	                              const char*           end,
	                              const unsigned char*  chars,
	                              const char*           _default = 0,
	                              bool                  negated  = false );
	
	const char* find_last_match( const char*           p,
	                             const char*           end,
	                             const unsigned char*  chars,
	                             const char*           _default = 0,
	                             bool                  negated  = false );
	
	
	inline const char* find_first_nonmatch( const char*  p,
	                                        const char*  end,
	                                        char         c,
	                                        const char*  _default = 0 )
	{
		return find_first_match( p, end, c, _default, true );
	}
	
	inline const char* find_last_nonmatch( const char*  p,
	                                       const char*  end,
	                                       char         c,
	                                       const char*  _default = 0 )
	{
		return find_last_match( p, end, c, _default, true );
	}
	
	inline const char* find_first_nonmatch( const char*           p,
	                                        const char*           end,
	                                        const unsigned char*  chars,
	                                        const char*           _default = 0 )
	{
		return find_first_match( p, end, chars, _default, true );
	}
	
	inline const char* find_last_nonmatch( const char*           p,
	                                       const char*           end,
	                                       const unsigned char*  chars,
	                                       const char*           _default = 0 )
	{
		return find_last_match( p, end, chars, _default, true );
	}
	
	
	const char* find_first_match( const char*  p,
	                              const char*  end,
	                              const char*  sub,
	                              unsigned     sub_length,
	                              const char*  _default = 0 );
	
	const char* find_last_match( const char*  p,
	                             const char*  end,
	                             const char*  sub,
	                             unsigned     sub_length,
	                             const char*  _default = 0 );
	
	
	inline const char* find_first_match( const char*  p,
	                                     unsigned     length,
	                                     char         c,
	                                     const char*  _default = 0,
	                                     bool         negated  = false )
	{
		return find_first_match( p, p + length, c, _default );
	}
	
	inline const char* find_last_match( const char*  p,
	                                    unsigned     length,
	                                    char         c,
	                                    const char*  _default = 0,
	                                    bool         negated  = false )
	{
		return find_last_match( p, p + length, c, _default );
	}
	
	inline const char* find_first_match( const char*           p,
	                                     unsigned              length,
	                                     const unsigned char*  chars,
	                                     const char*           _default = 0,
	                                     bool                  negated  = false )
	{
		return find_first_match( p, p + length, chars, _default );
	}
	
	inline const char* find_last_match( const char*           p,
	                                    unsigned              length,
	                                    const unsigned char*  chars,
	                                    const char*           _default = 0,
	                                    bool                  negated  = false )
	{
		return find_last_match( p, p + length, chars, _default );
	}
	
	
	inline const char* find_first_nonmatch( const char*  p,
	                                        unsigned     length,
	                                        char         c,
	                                        const char*  _default = 0 )
	{
		return find_first_match( p, p + length, c, _default, true );
	}
	
	inline const char* find_last_nonmatch( const char*  p,
	                                       unsigned     length,
	                                       char         c,
	                                       const char*  _default = 0 )
	{
		return find_last_match( p, p + length, c, _default, true );
	}
	
	inline const char* find_first_nonmatch( const char*           p,
	                                        unsigned              length,
	                                        const unsigned char*  chars,
	                                        const char*           _default = 0 )
	{
		return find_first_match( p, p + length, chars, _default, true );
	}
	
	inline const char* find_last_nonmatch( const char*           p,
	                                       unsigned              length,
	                                       const unsigned char*  chars,
	                                       const char*           _default = 0 )
	{
		return find_last_match( p, p + length, chars, _default, true );
	}
	
	
	inline const char* find_first_match( const char*  p,
	                                     unsigned     length,
	                                     const char*  sub,
	                                     unsigned     sub_length,
	                                     const char*  _default = 0 )
	{
		return find_first_match( p, p + length, sub, sub_length, _default );
	}
	
	inline const char* find_last_match( const char*  p,
	                                    unsigned     length,
	                                    const char*  sub,
	                                    unsigned     sub_length,
	                                    const char*  _default = 0 )
	{
		return find_last_match( p, p + length, sub, sub_length, _default );
	}
	
}

#endif
