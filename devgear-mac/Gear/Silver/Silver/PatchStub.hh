/*
	
	Patch.hh
	
	Joshua Juran
	
*/

#ifndef SILVER_PATCHSTUB_HH
#define SILVER_PATCHSTUB_HH

#include "Silver/CurrentA4.hh"


namespace Silver
{
	
	template < class PatchProcPtr, PatchProcPtr foo > class PatchStub;
	
	template < class R, R (*patch)( R(*)() ) >
	class PatchStub< R (*)( R(*)() ), patch >
	{
		protected:
			typedef pascal R (*ProcPtr)();
			
			static ProcPtr& NextHandler()
			{
				static ProcPtr nextHandler = NULL;
				
				return nextHandler;
			}
		
		public:
			static pascal R Function()
			{
				CurrentA4 a4;
				
				return patch( NextHandler() );
			}
	};
	
	template < class R, class P0, R (*patch)( P0, R(*)(P0) ) >
	class PatchStub< R (*)( P0, R(*)(P0) ), patch >
	{
		protected:
			typedef pascal R (*ProcPtr)( P0 );
			
			static ProcPtr& NextHandler()
			{
				static ProcPtr nextHandler = NULL;
				
				return nextHandler;
			}
		
		public:
			static pascal R Function( P0 p0 )
			{
				CurrentA4 a4;
				
				return patch( p0, NextHandler() );
			}
	};
	
	template < class R, class P0, class P1, R (*patch)( P0, P1, R(*)(P0, P1) ) >
	class PatchStub< R (*)( P0, P1, R(*)(P0, P1) ), patch >
	{
		protected:
			typedef pascal R (*ProcPtr)( P0, P1 );
			
			static ProcPtr& NextHandler()
			{
				static ProcPtr nextHandler = NULL;
				
				return nextHandler;
			}
		
		public:
			static pascal R Function( P0 p0, P1 p1 )
			{
				CurrentA4 a4;
				
				return patch( p0, p1, NextHandler() );
			}
	};
	
	template < class R, class P0, class P1, class P2, R (*patch)( P0, P1, P2, R(*)(P0, P1, P2) ) >
	class PatchStub< R (*)( P0, P1, P2, R(*)(P0, P1, P2) ), patch >
	{
		protected:
			typedef pascal R (*ProcPtr)( P0, P1, P2 );
			
			static ProcPtr& NextHandler()
			{
				static ProcPtr nextHandler = NULL;
				
				return nextHandler;
			}
		
		public:
			static pascal R Function( P0 p0, P1 p1, P2 p2 )
			{
				CurrentA4 a4;
				
				return patch( p0, p1, p2, NextHandler() );
			}
	};
	
}

#endif

