// sockaddr.hh
// -----------
//
// Maintained by Joshua Juran

// Part of the Nitrogen project.
//
// Written 2008 by Joshua Juran.
//
// This code was written entirely by the above contributor, who places it
// in the public domain.


#ifndef POSEVEN_TYPES_SOCKADDR_HH
#define POSEVEN_TYPES_SOCKADDR_HH

// POSIX
#include <sys/socket.h>

// nucleus
#ifndef NUCLEUS_ENUMERATIONTRAITS_HH
#include "nucleus/enumeration_traits.hh"
#endif


namespace poseven
{
	
	enum address_family
	{
		af_unspec    = AF_UNSPEC,
		af_unix      = AF_UNIX,
		af_inet      = AF_INET,
		
	#ifdef AF_IPX
		
		af_ipx       = AF_IPX,
		
	#endif
		
		af_appletalk = AF_APPLETALK,
		
		address_family_max = nucleus::enumeration_traits< unsigned short >::max
	};
	
	template < address_family family > struct sockaddr_traits;
	
}

#endif

