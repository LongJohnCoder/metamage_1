/*
	get_option.hh
	-------------
*/

#ifndef COMMAND_GETOPTION_HH
#define COMMAND_GETOPTION_HH


namespace command
{
	
	namespace constants
	{
		
		enum
		{
			Option_undefined     = -1,
			Option_param_missing = -2,
			Option_param_bogus   = -3,
		};
		
		enum
		{
			Param_unwanted,
			Param_optional,
			Param_required,
			Param_required_can_be_array,
		};
		
		enum
		{
			Flag_return_errors = 0x0001,
		};
		
	}
	
	using namespace constants;
	
	struct option
	{
		const char*  name;
		short        code;
		short        mode;  // parameter mode
	};
	
	struct option_result
	{
		char*  param;
		char*  mark;
		
		short  array_offset;
		short  array_length;
	};
	
	extern option_result global_result;
	
	short get_option_( char* const*&   argv,
	                   const option*   options,
	                   int             flags,
	                   option_result&  result );
	
	inline short get_option( char* const**   argvp,
	                         const option*   options,
	                         int             flags = 0,
	                         option_result*  result = &global_result )
	{
		if ( argvp == 0 )  return 0;  // NULL (argvp)
		
		return get_option_( *argvp, options, flags, *result );
	}
	
}

#endif
