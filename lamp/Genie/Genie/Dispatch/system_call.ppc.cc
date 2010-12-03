/*
	system_call.ppc.cc
	------------------
*/

#include "Genie/Dispatch/system_call.ppc.hh"

// Genie
#include "Genie/SystemCallRegistry.hh"
#include "Genie/Dispatch/kernel_boundary.hh"


namespace Genie
{
	
#ifdef __POWERPC__
	
	enum { kSystemCallSize = sizeof (SystemCall) };
	
	asm void dispatch_ppc_system_call( ... )
	{
		// allocate a stack frame
		stwu	SP,-64(SP)
		
		// save up to 6 parameters
		stw		r5,32(SP)
		stw		r6,36(SP)
		stw		r7,40(SP)
		stw		r8,44(SP)
		stw		r3,48(SP)
		stw		r4,52(SP)
		
		// save the system call number
		stw		r11,56(SP)
		
		// load syscall number and params as arguments
		mr		r3,r11
		addi	r4,SP,32
		
		bl		enter_system_call
		
		// restore system call number
		lwz		r11,56(SP)
		
		// restore parameters
		lwz		r5,32(SP)
		lwz		r6,36(SP)
		lwz		r7,40(SP)
		lwz		r8,44(SP)
		lwz		r3,48(SP)
		lwz		r4,52(SP)
		
		// r11 contains the requested system call number
		// r3-r8 are up to 6 arguments
		
		// r12.last = gLastSystemCall;
		lwz		r12,gLastSystemCall
		lwz		r12,0(r12)
		
		// if ( r11.index > r12.last )
		cmpl	cr0,r11,r12
		blt+	cr0,in_range
		// r11.index = r12.last;
		mr		r11,r12
		
	in_range:
		
		// r12.array = gSystemCallArray;
		lwz		r12,gSystemCallArray
		lwz		r12,0(r12)
		
		// r12.f = r12.array[ r11.index ].function
		mulli	r11,r11,kSystemCallSize
		add		r12,r12,r11
		lwz		r12,0(r12)
		
		// load system call address for local jump
		lwz		r0,0(r12)
		
		// jump to system call
		mtctr	r0
		bctrl
		
		// save result
		stw		r3,32(SP)
		
		bl		leave_system_call
		
		// restore result
		lwz		r3,32(SP)
		
		// deallocate our stack frame
		addi	SP,SP,64
		
		// restore RTOC
		lwz		RTOC,20(SP)
	}
	
#endif
	
}

