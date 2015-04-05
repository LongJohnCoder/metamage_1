// sockaddr_in.hh
// --------------
//
// Maintained by Joshua Juran

// Part of the Nitrogen project.
//
// Written 2008 by Joshua Juran.
//
// This code was written entirely by the above contributor, who places it
// in the public domain.


#ifndef POSEVEN_TYPES_SOCKADDR_IN_HH
#define POSEVEN_TYPES_SOCKADDR_IN_HH

// poseven
#ifndef POSEVEN_TYPES_INADDR_T_HH
#include "poseven/types/in_addr_t.hh"
#endif
#ifndef POSEVEN_TYPES_INPORT_T_HH
#include "poseven/types/in_port_t.hh"
#endif
#ifndef POSEVEN_TYPES_SOCKADDR_HH
#include "poseven/types/sockaddr.hh"
#endif


namespace poseven
{
	
	template <> struct sockaddr_traits< af_inet >
	{
		typedef sockaddr_in address_type;
		
		static address_type make( in_addr_t addr, big_endian::in_port_t port )
		{
			struct sockaddr_in inet_address = { 0 };
			
			inet_address.sin_family      = af_inet;
			inet_address.sin_port        = port;
			inet_address.sin_addr.s_addr = addr;
			
			return inet_address;
		}
		
	#if CONFIG_LITTLE_ENDIAN
		
		static address_type make( in_addr_t addr, in_port_t port )
		{
			return make( addr, big_endian::in_port_t( htons( port ) ) );
		}
		
	#endif
	};
	
}

#endif
