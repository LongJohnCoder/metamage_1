/*	==============
 *	SystemCalls.cc
 *	==============
 */

// Mac OS
#include <LowMem.h>

// Standard C/C++
#include <cerrno>

// Lamp
#include "lamp/syscalls.h"

// Kerosene
#include "FreeTheMallocPool.h"


#pragma exceptions off


typedef void (*CleanupHandler)();


#if TARGET_CPU_68K
	
	enum { kDispatcherAddr = (long) LMGetToolScratch() };
	
	static asm void SystemCall()
	{
		MOVEA.L		kDispatcherAddr,A0	;  // load the dispatcher's address
										;  // arg 1:  syscall index already on stack
		JMP			(A0)				;  // jump to dispatcher -- doesn't return
		
		// Not reached
	}
	
	#define DEFINE_STUB( name )          \
		extern "C" void name();          \
		asm void name()                  \
		{                                \
			MOVE.L #__NR_##name,-(SP) ;  \
			JMP SystemCall            ;  \
		}
	
	
	asm static void InitProc( CleanupHandler, int* )
	{
		MOVE.L #__NR_InitProc,-(SP) ;
		JMP SystemCall              ;
	}
	
#endif

#if TARGET_CPU_PPC
	
	static void* const gDispatcher = *reinterpret_cast< void** >( ::LMGetToolScratch() );
	
	extern "C" void __ptr_glue();
	
	static asm void SystemCall()
	{
		mflr	r0				// get caller's return address
		stw		r0,8(SP)		// store return address in caller's stack frame
		
		mr		r0,r11			// copy syscall number into r0
		
		stwu	SP,-64(SP)		// allocate our own stack frame
		
		lwz		r12,gDispatcher	// load dispatcher T-vector
		
		bl		__ptr_glue		// cross-TOC call
		nop						// synchronize pipeline
		lwz		RTOC,20(SP)		// restore our RTOC
		
		addi	SP,SP,64		// deallocate our stack frame
		
		lwz		r0,8(SP)		// reload caller's return address
		mtlr	r0				// load it into the link register
		blr						// return
	}
	
	#define DEFINE_STUB( name )   \
		extern "C" void name();   \
		asm void name()           \
		{                         \
			li r11,__NR_##name ;  \
			b SystemCall       ;  \
		}
	
	
	asm static void InitProc( CleanupHandler, int* )
	{
		li r11,__NR_InitProc ;
		b SystemCall         ;
	}
	
#endif


class Initializer
{
	public:
		Initializer()
		{
			InitProc( TARGET_RT_MAC_CFM ? NULL : &FreeTheMallocPool, &errno );
		}
};

static Initializer gInitializer;


DEFINE_STUB( _exit )
DEFINE_STUB( SpawnVFork )
DEFINE_STUB( read  )
DEFINE_STUB( write )
DEFINE_STUB( open  )
DEFINE_STUB( close )
DEFINE_STUB( waitpid )
DEFINE_STUB( realpath_k )

DEFINE_STUB( unlink )
DEFINE_STUB( execve )
DEFINE_STUB( chdir  )

DEFINE_STUB( mknod )
DEFINE_STUB( chmod )

DEFINE_STUB( getcwd_k )
DEFINE_STUB( lseek )
DEFINE_STUB( getpid )

DEFINE_STUB( ptrace )
DEFINE_STUB( alarm )

DEFINE_STUB( pause )

DEFINE_STUB( access )

DEFINE_STUB( sync )
DEFINE_STUB( kill )
DEFINE_STUB( rename )
DEFINE_STUB( mkdir )
DEFINE_STUB( rmdir )
DEFINE_STUB( dup )
DEFINE_STUB( pipe )
DEFINE_STUB( times )

DEFINE_STUB( signal )
DEFINE_STUB( flock )
DEFINE_STUB( ioctl )
DEFINE_STUB( fcntl )

DEFINE_STUB( setpgid )

DEFINE_STUB( uname )
DEFINE_STUB( umask )

DEFINE_STUB( dup2 )
DEFINE_STUB( getppid )

DEFINE_STUB( setsid )
DEFINE_STUB( sigaction )

DEFINE_STUB( sigsuspend )
DEFINE_STUB( sigpending )

DEFINE_STUB( gettimeofday )

DEFINE_STUB( select )
DEFINE_STUB( symlink )

DEFINE_STUB( readlink_k )

DEFINE_STUB( truncate )
DEFINE_STUB( ftruncate )
DEFINE_STUB( fchmod )

DEFINE_STUB( stat )
DEFINE_STUB( lstat )
DEFINE_STUB( fstat )

DEFINE_STUB( fsync )

DEFINE_STUB( sigprocmask )

DEFINE_STUB( getpgid )
DEFINE_STUB( fchdir )

DEFINE_STUB( getdents )

DEFINE_STUB( getsid )

DEFINE_STUB( nanosleep )

DEFINE_STUB( poll )

DEFINE_STUB( socketpair )
DEFINE_STUB( socket )
DEFINE_STUB( bind )
DEFINE_STUB( listen )
DEFINE_STUB( accept )
DEFINE_STUB( connect )
DEFINE_STUB( shutdown )
DEFINE_STUB( getsockname )
DEFINE_STUB( getpeername )
DEFINE_STUB( getsockopt )
DEFINE_STUB( setsockopt )
DEFINE_STUB( send )
DEFINE_STUB( recv )
DEFINE_STUB( sendto )
DEFINE_STUB( recvfrom )

DEFINE_STUB( gethostbyname )

DEFINE_STUB( fork_and_exit )

DEFINE_STUB( pump )

DEFINE_STUB( ttypair )

DEFINE_STUB( copyfile )

DEFINE_STUB( AESendBlocking )
DEFINE_STUB( OTInetMailExchange_k )

