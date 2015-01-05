#ifndef RELIX_PARAMETERBLOCK_H
#define RELIX_PARAMETERBLOCK_H

// Standard C
#include <stdint.h>

// POSIX
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _relix_user_parameter_block
{
	const void* stack_bottom;
	const void* stack_limit;
	
	int* errno_var;
	
	void (*cleanup)();
	
#ifdef __MC68K__
	
	void* globals;
	
#endif
};

typedef struct _relix_user_parameter_block _relix_user_parameter_block;


struct _relix_system_parameter_block
{
	_relix_user_parameter_block* current_user;
	
	const size_t system_block_length;
	const size_t user_block_length;
	
	void (*const dispatcher)( ... );
	
	uint64_t (*const microsecond_timer)();
};

typedef struct _relix_system_parameter_block _relix_system_parameter_block;

#ifdef __cplusplus
}
#endif

#endif
