/*
	t/get_option.cc
	---------------
*/

// Standard C
#include <string.h>

// command
#include "command/get_option.hh"

// tap-out
#include "tap/test.hh"


static const unsigned n_tests = 1 + 4 * 29;


enum
{
	Option_verbose  = 'v',
	Option_progress = 'p',
	Option_output   = 'o',
};

using namespace command::constants;

static command::option options[] =
{
	{ "verbose",  Option_verbose                  },
	{ "progress", Option_progress, Param_optional },
	{ "output",   Option_output,   Param_required },
	{ NULL }
};

struct test_case
{
	short              opt;
	short              delta;
	const char* const  argv[ 3 ];
	const char*        param;
};

static const test_case test_cases[] =
{
	{ 0, 0, { NULL } },
	{ 0, 0, { "", NULL } },
	{ 0, 0, { "foo", NULL } },
	{ 0, 0, { "-", NULL } },
	{ 0, 1, { "--", NULL } },
	{ 0, 1, { "--", "-v", NULL } },
	
	{ 'v', 1, { "-v", NULL } },
	{ 'p', 1, { "-p", NULL } },
	
	{ 'v', 1, { "-v", "foo", NULL } },
	{ 'p', 1, { "-p", "foo", NULL } },
	{ 'o', 2, { "-o", "foo", NULL }, "foo" },
	
	{ 'v', 2 << 4, { "-vpo", "foo", NULL } },
	{ 'p', 2 << 4, { "-pov", "foo", NULL } },
	{ 'o', 1,      { "-ovp", "foo", NULL }, "vp" },
	
	{ 'p', 2 << 8 | 3 << 4, { "-vpo", "foo", NULL } },
	{ 'o', 2 << 8 | 1,      { "-pov", "foo", NULL }, "v" },
	
	{ 'p', 2 << 8 | 1, { "-vp", "foo", NULL } },
	{ 'v', 2 << 8 | 1, { "-pv", "foo", NULL } },
	
	{ 'v', 1, { "--verbose",  "foo", NULL } },
	{ 'p', 1, { "--progress", "foo", NULL } },
	{ 'o', 2, { "--output",   "foo", NULL }, "foo" },
	
	{ 'p', 1, { "--progress=foo", NULL }, "foo" },
	{ 'o', 1, { "--output=foo", NULL }, "foo" },
	
	{ -1, 0, { "-x", NULL } },
	{ -2, 1, { "-o", NULL } },
	
	{ -2, 2 << 8 | 1, { "-vo", NULL } },
	
	{ -1, 0, { "--foobar", NULL } },
	{ -2, 1, { "--output", NULL } },
	{ -3, 0, { "--verbose=11", NULL }, "11" },
};

#define ARRAY_LEN( a )  (sizeof a / sizeof a[0])

static void null()
{
	short opt = command::get_option( NULL,
	                                 options,
	                                 Flag_return_errors );
	
	EXPECT( opt == 0 );
}

static void other()
{
	const test_case* end = test_cases + ARRAY_LEN( test_cases );
	
	for ( const test_case* t = test_cases;  t < end;  ++t )
	{
		command::option_result result = { 0 };
		
		if ( short start = t->delta >> 8 )
		{
			result.mark = *t->argv + start;
		}
		
		char* const* args = (char* const*) t->argv;
		
		short opt = command::get_option( &args,
		                                 options,
		                                 Flag_return_errors,
		                                 &result );
		
		EXPECT( opt == t->opt );
		
		short delta = args - (char* const*) t->argv;
		
		if ( result.mark )
		{
			delta |= (result.mark - *args) << 4;
		}
		
		EXPECT( delta == (t->delta & 0xFF) );
		
		EXPECT( (result.param != 0) == (t->param != 0) );
		
		if ( result.param  &&  t->param )
		{
			EXPECT( strcmp( result.param, t->param ) == 0 );
		}
		else
		{
			EXPECT( true );
		}
	}
}

int main( int argc, const char *const *argv )
{
	tap::start( "get_option", n_tests );
	
	null();
	
	other();
	
	return 0;
}
