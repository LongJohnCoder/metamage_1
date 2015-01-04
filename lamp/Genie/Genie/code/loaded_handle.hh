/*
	loaded_handle.hh
	----------------
*/

#ifndef GENIE_CODE_LOADEDHANDLE_HH
#define GENIE_CODE_LOADEDHANDLE_HH

// vfs-relix
#include "vfs/program.hh"

// Genie
#include "Genie/code/execution_unit.hh"


namespace Genie
{
	
	class loaded_handle : public vfs::program
	{
		private:
			execution_unit its_execution_unit;
		
		public:
			loaded_handle( const execution_unit& exec )
			:
				its_execution_unit( exec )
			{
			}
			
			const execution_unit& unit() const
			{
				return its_execution_unit;
			}
	};
	
}

#endif
