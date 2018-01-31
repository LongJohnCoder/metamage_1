/*
	breathe.cc
	----------
*/

// Standard C
#include <time.h>

// relix-kernel
#include "relix/api/breathe.hh"
#include "relix/task/scheduler.hh"

// Genie
#include "Genie/current_process.hh"
#include "Genie/Process.hh"
#include "Genie/api/signals.hh"


namespace relix
{
	
	static clock_t the_last_breath_time;
	
	
	bool breathe( bool may_throw )
	{
		using namespace Genie;
		
		if ( check_signals( may_throw ) )
		{
			return true;
		}
		
		const clock_t now = clock();
		
		Process& current = current_process();
		
		if ( now - the_last_breath_time > 20000 )
		{
			relix::mark_thread_active( current_process().gettid() );
			
			current.Breathe();
			
			relix::mark_thread_inactive( current_process().gettid() );
			
			the_last_breath_time = clock();
			
			// Check for fatal signals again
			return check_signals( may_throw );
		}
		
		return false;
	}
	
}
