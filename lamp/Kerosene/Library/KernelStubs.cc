/*	==============
 *	KernelStubs.cc
 *	==============
 */

// POSIX
#include "netdb.h"

// Mac OS
#include <OpenTransportProviders.h>

// Kerosene
#include "SystemCalls.hh"


#pragma exceptions off


#if 0
#pragma export on
#endif
	
	#pragma mark -
	#pragma mark � netdb �
	
	void endhostent()
	{
	}
	
	void endprotoent()
	{
	}
	
	void endservent()
	{
	}
	
	/*
	struct hostent* gethostbyaddr( const void*, size_t, int )
	{
		return NULL;
	}
	*/
	
	struct hostent* gethostent()
	{
		return NULL;
	}
	
	struct protoent* getprotobyname( const char* name )
	{
		return NULL;
	}
	
	struct protoent* getprotobynumber( int )
	{
		return NULL;
	}
	
	struct protoent* getprotoent()
	{
		return NULL;
	}
	
	struct servent* getservbyname( const char*, const char* )
	{
		return NULL;
	}
	
	struct servent* getservbyport( int, const char* )
	{
		return NULL;
	}
	
	struct servent* getservent()
	{
		return NULL;
	}
	
	void herror( const char* )
	{
	}
	
	/*
	char* hstrerror( int )
	{
		return "";
	}
	*/
	
	void setprotoent( int )
	{
	}
	
	void setservent( int )
	{
	}
	
	// Override ::OTInetMailExchange() to call into the kernel, so we can use
	// our own SharedOpenTransport and InetSvcRef.
	pascal OSStatus OTInetMailExchange( InetSvcRef, char* domain, UInt16* count, InetMailExchange* result )
	{
		return OTInetMailExchange_k( domain, count, result );
	}
	
	#pragma mark -
	#pragma mark � Genie �
	
	extern "C" OSStatus Path2FSSpec( const char* pathname, FSSpec* outFSS );
	
	FSSpec Path2FSS( const char* pathname )
	{
		FSSpec spec;	
		
		OSStatus err = Path2FSSpec( pathname, &spec );
		
		if ( err != noErr )
		{
			spec.vRefNum = 0x8000;
			spec.parID = err;
			spec.name[0] = 0;
		}
		
		return spec;
	}
	
#if 0	
#pragma export reset
#endif

