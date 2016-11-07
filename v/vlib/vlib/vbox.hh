/*
	vbox.hh
	-------
*/

#ifndef VLIB_VBOX_HH
#define VLIB_VBOX_HH

// Standard C
#include <stdint.h>

// plus
#include "plus/datum_storage.hh"
#include "plus/ibox.hh"


#if defined( __MWERKS__ )  &&  defined( __POWERPC__ )
/*
	This works around a compiler bug.  MWCPPC 2.4.1, with optimizations on,
	is testing the vbox's `type` byte (to determine if it later needs to call
	destroy_extent()) *before* constructing the vbox, when storing the result
	of a function returning a Value onto the stack as a const Value.
	
	We're not limiting the workaround based on CONFIG_DEBUGGING, because I'm
	not certain exactly which compiler options trigger the failure mode, and
	we can't rule out certain optimizations being switched on for debug builds
	(which are already using -O2 with Metrowerks C/C++).  If a new build tool
	is developed that mistakenly reverses CONFIG_DEBUGGING or specifies the
	wrong optimization level, the consequences shouldn't include a broken V
	interpreter -- especially since the next build tool will in all likelihood
	be written in V.  Anyway, an examination of the object code generated for
	a debug build of vc's main.cc reveals that it makes no difference.
	
	We also have to do this for the `semantics` byte, for the same reason.
	There, the symptom is spurious "arguments don't match function prototype"
	exceptions.
*/

	typedef volatile char Char;

#else

	typedef char Char;

#endif

namespace vlib
{
	
	typedef void (*destructor)( void* );
	
	enum vbox_type
	{
		Vbox_alloc = -1,
		Vbox_empty,
		Vbox_bool,
		Vbox_i8,
		Vbox_u8,
		Vbox_i16,
		Vbox_u16,
		Vbox_i32,
		Vbox_u32,
		Vbox_i64,
		Vbox_u64,
		Vbox_bigint,
		Vbox_string,
		Vbox_pointer,
	};
	
	typedef plus::datum_allocation  vu_string;
	typedef plus::ibox_structure    vu_ibox;
	
	struct vu_alloc : vu_string
	{
		// Char is a volatile char.  See above for rationale.
		
		Char  type;       // vbox_type
		Char  semantics;  // value_type
	};
	
	struct vu_chars
	{
		signed char data[ sizeof (vu_string) ];
	};
	
	union vu_ints
	{
		uint64_t  u64;
		int64_t   i64;
		uint32_t  u32;
		int32_t   i32;
		uint16_t  u16;
		int16_t   i16;
		uint8_t   u8;
		int8_t    i8;
		bool      boolean;
	};
	
	union vu
	{
		vu_alloc   alloc;
		vu_string  string;
		vu_chars   chars;
		vu_ibox    ibox;
		vu_ints    ints;
	};
	
	class vbox
	{
		private:
			vu u;
		
		private:
			void destroy_extent();
			
			void destroy()
			{
				if ( has_extent() )
				{
					destroy_extent();
				}
			}
		
		public:
			~vbox()  { destroy(); }
			
			vbox( char semantics = 0 )
			{
				u.alloc.type      = Vbox_empty;
				u.alloc.semantics = semantics;
			}
			
			vbox( bool x, char semantics )
			{
				u.ints.boolean    = x;
				u.alloc.type      = Vbox_bool;
				u.alloc.semantics = semantics;
			}
			
			vbox( int8_t x, char semantics )
			{
				u.ints.i8         = x;
				u.alloc.type      = Vbox_i8;
				u.alloc.semantics = semantics;
			}
			
			vbox( uint8_t x, char semantics )
			{
				u.ints.u8         = x;
				u.alloc.type      = Vbox_u8;
				u.alloc.semantics = semantics;
			}
			
			vbox( int16_t x, char semantics )
			{
				u.ints.i16        = x;
				u.alloc.type      = Vbox_i16;
				u.alloc.semantics = semantics;
			}
			
			vbox( uint16_t x, char semantics )
			{
				u.ints.u16        = x;
				u.alloc.type      = Vbox_u16;
				u.alloc.semantics = semantics;
			}
			
			vbox( int32_t x, char semantics )
			{
				u.ints.i32        = x;
				u.alloc.type      = Vbox_i32;
				u.alloc.semantics = semantics;
			}
			
			vbox( uint32_t x, char semantics )
			{
				u.ints.u32        = x;
				u.alloc.type      = Vbox_u32;
				u.alloc.semantics = semantics;
			}
			
			vbox( int64_t x, char semantics )
			{
				u.ints.i64        = x;
				u.alloc.type      = Vbox_i64;
				u.alloc.semantics = semantics;
			}
			
			vbox( uint64_t x, char semantics )
			{
				u.ints.u64        = x;
				u.alloc.type      = Vbox_u64;
				u.alloc.semantics = semantics;
			}
			
			vbox( const vu_ibox& ix, char semantics );
			
			vbox( const vu_string& s, char semantics );
			
			vbox( const void* p, char semantics )
			{
				u.alloc.pointer   = (char*) p;
				u.alloc.type      = Vbox_pointer;
				u.alloc.semantics = semantics;
			}
			
			vbox( unsigned long n, destructor dtor, char semantics );
			
			vbox( const vbox& that );
			
			vbox& operator=( const vbox& that );
			
			void swap( vbox& b );
			
			const void* transfer_extent();
			
			unsigned long refcount() const;
			
			bool has_extent() const  { return int8_t( u.alloc.type ) < 0; }
			
			char semantics() const  { return u.alloc.semantics; }
			
			bool boolean() const  { return u.ints.boolean; }
			
			int8_t   i8()  const  { return u.ints.i8;  }
			uint8_t  u8()  const  { return u.ints.u8;  }
			int16_t  i16() const  { return u.ints.i16; }
			uint16_t u16() const  { return u.ints.u16; }
			int32_t  i32() const  { return u.ints.i32; }
			uint32_t u32() const  { return u.ints.u32; }
			int64_t  i64() const  { return u.ints.i64; }
			uint64_t u64() const  { return u.ints.u64; }
			
			const void* pointer() const  { return u.alloc.pointer; }
			
			void unshare();
			
			friend unsigned long area( const vbox& box );
	};
	
	inline void swap( vbox& a, vbox& b )
	{
		a.swap( b );
	}
	
	unsigned long area( const vbox& box );
	
}

#endif
