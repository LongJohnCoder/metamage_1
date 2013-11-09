/*
	userland.hh
	-----------
*/

#ifndef GENIE_USERLAND_HH
#define GENIE_USERLAND_HH


namespace Genie
{
	
#ifdef __MC68K__
	
	static inline asm void* GetA4()
	{
		MOVEA.L A4,A0
	}
	
	static inline asm void* SetA4( void* : __a0 )
	{
		EXG A0,A4
	}
	
	void call_signal_handler( void* handler : __a0, int signo : __d0 );
	
	#define ENTER_USERMAIN()  void* _saved_a4 = GetA4()
	#define EXIT_USERMAIN()   (void)            SetA4( _saved_a4 )
	
	#define ENTER_USERLAND( globals )  void* _saved_a4 = SetA4( globals )
	#define EXIT_USERLAND()            (void)            SetA4( _saved_a4 )
	
#else
	
	inline void call_signal_handler( void (*handler)(int), int signo )
	{
		handler( signo );
	}
	
	#define ENTER_USERMAIN()  /**/
	#define EXIT_USERMAIN()   /**/
	
	#define ENTER_USERLAND( globals )  /**/
	#define EXIT_USERLAND()            /**/
	
#endif
	
}

#endif

