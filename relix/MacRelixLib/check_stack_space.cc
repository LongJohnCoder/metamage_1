/*
	check_stack_space.cc
	--------------------
*/

// relix-api
#include "relix/api/check_stack_space.hh"

// POSIX
#include <signal.h>

// cthread-either
#include "cthread-either.hh"

// relix-api
#include "relix/api/deliver_fatal_signal.hh"


#ifndef SIGSTKFLT
#define SIGSTKFLT  0
#endif


namespace relix
{
	
	void check_stack_space()
	{
		using cthread::either::current_thread_stack_space;
		
		const unsigned long space = current_thread_stack_space();
		
		// space will be 0 if we're not on a Thread Manager stack
		
		if ( space != 0  &&  space < 8192 )
		{
			deliver_fatal_signal( SIGSTKFLT );
		}
	}
	
}
