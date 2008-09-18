/*	=============
 *	StackCrawl.hh
 *	=============
 */

#ifndef BACKTRACE_STACKCRAWL_HH
#define BACKTRACE_STACKCRAWL_HH

#if PRAGMA_ONCE
#pragma once
#endif

// Standard C++
#include <vector>


namespace Backtrace
{
	
	// The 'native' code type, whatever that is.
	// Most platforms have just one, but Mac OS is special:
	// It's either classic 68K or Mach-O -- never CFM.
	typedef const struct OpaqueCodeNative*  ReturnAddrNative;
	
#if defined( __MACOS__ ) && !defined( __MACH__ )
	
	typedef const struct OpaqueCodeCFM*  ReturnAddrCFM;
	
#endif
	
	
#if defined( __MC68K__ )  ||  defined( __MACOS__ ) && !defined( __MACH__ )
	
	// 68K is considered native for traditional Mac OS
	// Even Carbon CFM binaries may run on OS 9 and can intermingle with 68K
	typedef ReturnAddrNative ReturnAddr68K;
	
#else
	
	typedef const struct OpaqueCodeAlien68K* ReturnAddr68K;
	
#endif
	
	
#if defined( __MACOS__ ) && !defined( __MACH__ )
	
	typedef ReturnAddrCFM ReturnAddrPPC;
	
#elif defined( __POWERPC__ )
	
	typedef ReturnAddrNative ReturnAddrPPC;
	
#else
	
	typedef const struct OpaqueCodeAlienPPC* ReturnAddrPPC;
	
#endif
	
	
#ifdef __i386__
	
	typedef ReturnAddrNative ReturnAddrX86;
	
#else
	
	typedef const struct OpaqueCodeAlienX86* ReturnAddrX86;
	
#endif
	
	
#ifdef __MACH__
	
	typedef ReturnAddrNative ReturnAddrMachO;
	
#else
	
	typedef const struct OpaqueCodeAlienMachO* ReturnAddrMachO;
	
#endif


#if ( defined( __POWERPC__ ) || defined( __MACOS__ ) )  &&  !defined( __MACH__ )
	
	typedef ReturnAddrPPC ReturnAddrWithTraceback;
	
#else
	
	typedef const struct OpaqueCodeAlienTraceback* ReturnAddrWithTraceback;
	
#endif
	
	
	typedef const struct OpaqueStackFrame* StackFramePtr;
	
	struct ReturnAddress
	{
		union
		{
			ReturnAddrNative  addrNative;
			
		#if defined( __MACOS__ ) && !defined( __MACH__ )
			
			ReturnAddrCFM     addrCFM;
			
		#endif
		};
		
		bool isCFM;
		
		ReturnAddress() : addrNative(), isCFM()  {}
		
		ReturnAddress( ReturnAddrNative  addr ) : addrNative( addr ), isCFM( false  )  {}
		
	#if defined( __MACOS__ ) && !defined( __MACH__ )
		
		ReturnAddress( ReturnAddrCFM     addr ) : addrCFM   ( addr ), isCFM( true  )  {}
		
	#endif
	};
	
	
	StackFramePtr GetStackFramePointer( int levelsToSkip = 0 );
	
	std::vector< ReturnAddress > MakeStackCrawlFromTopToBottom( StackFramePtr frame, const void* limit );
	
	inline std::vector< ReturnAddress > MakeStackCrawlFromTop( StackFramePtr frame )
	{
		return MakeStackCrawlFromTopToBottom( frame, (const void*) 0xFFFFFFFF );
	}
	
	std::vector< ReturnAddress > MakeStackCrawlToBottom( const void* limit );
	
	inline std::vector< ReturnAddress > MakeStackCrawl()
	{
		return MakeStackCrawlToBottom( (const void*) 0xFFFFFFFF );
	}
	
}

#endif

