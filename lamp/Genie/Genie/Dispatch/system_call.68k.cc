/*
	system_call.68k.cc
	------------------
*/

#include "Genie/Dispatch/system_call.68k.hh"

// Relix
#include "relix/config/syscall_stacks.hh"

// Genie
#include "Genie/SystemCallRegistry.hh"
#include "Genie/Dispatch/kernel_boundary.hh"


extern "C" void* current_stack_base();


namespace Genie
{
	
#ifdef __MC68K__
	
	asm void dispatch_68k_system_call( ... )
	{
		// D0 contains the system call number
		
		MOVEA.L  SP,A1
		
		LINK     A6,#0
		
		MOVE.L   A1,-(SP)  // push the address of the first arg
		MOVE.L   D0,-(SP)  // push the system call number
		
	#if CONFIG_SYSCALL_STACKS
		
		JSR      current_stack_base
		
		// copy things from current stack to new stack
		
		SUBQ     #8,A0  // point after return address, to stack limit
		
		MOVE.L   4(A6),-(A0)  // return address
		MOVE.L    (A6),-(A0)  // saved frame pointer (backlink)
		
		MOVE.L   4(SP),-(A0)  // address of the first arg
		MOVE.L    (SP),-(A0)  // system call number
		
		MOVEA.L  A0,SP  // switch to new stack
		
	#endif
		
	restart:
		
		JSR      enter_system_call
		
		MOVE.L   (SP),D0  // restore D0
		
		CMP.W    gLastSystemCall,D0
		BLT      in_range
		
		MOVE.W   gLastSystemCall,D0
		
	in_range:
		MOVEA.L  gSystemCallArray,A0
		
	#if __MC68020__
		MOVEA.L  (A0,D0.W*8),A0
	#else
		LSL.W    #3,D0
		MOVEA.L  (A0,D0.W),A0
	#endif
		
		MOVE.L   28(A6),-(SP)
		MOVE.L   24(A6),-(SP)
		MOVE.L   20(A6),-(SP)
		MOVE.L   16(A6),-(SP)
		MOVE.L   12(A6),-(SP)
		MOVE.L    8(A6),-(SP)
		
		JSR      (A0)
		
		ADDA.L   #24,SP
		
		MOVE.L   D0,-(SP)  // save D0
		
		JSR      leave_system_call
		
		MOVE.L   D0,D1  // restart the system call?
		
		MOVE.L   (SP)+,D0  // restore D0
		
		TST.L    D1  // restart the system call?
		
		BNE.S    restart
		
		UNLK     A6
		
		RTS
	}
	
#endif
	
}
