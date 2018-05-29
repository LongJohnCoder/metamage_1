/*
	syscalls.68k.c.h
	----------------
*/

#ifdef __MWERKS__
	
	asm int syscall( int number, ... )
	{
		MOVE.L  4(SP), D0    // copy system call number to d0
		MOVE.L  (SP)+, (SP)  // overwrite it with the return address, and pop
		
		TRAP  #2
	}
	
	#define DEFINE_SYSCALL_7F( name ) \
		extern void name();           \
		asm void name()               \
		{                             \
			MOVEQ #__NR_##name,D0  ;  \
			TRAP  #2               ;  \
		}
	
	#define DEFINE_SYSCALL( name )    \
		extern void name();           \
		asm void name()               \
		{                             \
			MOVE.W #__NR_##name,D0 ;  \
			TRAP  #2               ;  \
		}
	
#else
	
	#define DEFINE_SYSCALL_7F( name )   \
		extern void name( void );       \
		void name( void )               \
		{                               \
			asm( 0x7000 + __NR_##name,  \
			     0x4e42 );              \
		}

	#define DEFINE_SYSCALL( name )     \
		void name( void )              \
		{                              \
			asm( 0x303c, __NR_##name,  \
			     0x4e42 );             \
		}

#endif
