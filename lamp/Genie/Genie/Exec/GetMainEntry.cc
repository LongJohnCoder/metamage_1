/*	===============
 *	GetMainEntry.cc
 *	===============
 */

#include "Genie/Exec/GetMainEntry.hh"

// Nitrogen
#include "Nitrogen/CodeFragments.h"

// Genie
#include "Genie/Exec/MainEntryPoint.hh"


namespace Genie
{
	
	namespace N = Nitrogen;
	namespace NN = Nucleus;
	
	
	class AddressMain : public MainEntryPoint
	{
		private:
			Standard_Entry itsEntry;
		
		public:
			AddressMain( Standard_Entry entry ) : itsEntry( entry )
			{
				ASSERT( itsEntry != NULL );
			}
			
			AddressMain( Trivial_Entry entry ) : itsEntry( (Standard_Entry) entry )
			{
				ASSERT( itsEntry != NULL );
			}
			
			void Invoke( int argc, iota::argv_t argv, iota::environ_t envp )
			{
				ASSERT( itsEntry != NULL );
				
				itsEntry( argc, argv );
			}
	};
	
	
	class BinaryImageClient
	{
		private:
			BinaryImage itsBinaryImage;
		
		public:
			BinaryImageClient( const BinaryImage& image ) : itsBinaryImage( image )
			{
			}
	};
	
	class CodeResourceMain : public BinaryImageClient, public AddressMain
	{
		public:
			CodeResourceMain( const BinaryImage& image ) : BinaryImageClient( image ),
			                                               AddressMain( reinterpret_cast< Standard_Entry >( *image.Get().Get() ) )
			{
			}
	};
	
	
	inline NN::Owned< N::CFragConnectionID > ConnectToFragment( const BinaryImage& image )
	{
		// Set up dispatcher and envp
		
		return N::GetMemFragment< N::kPrivateCFragCopy >( *image.Get(),
		                                                  N::GetHandleSize( image ) );
	}
	
	
	static void LoadSymbol( N::CFragConnectionID fragment, ConstStr255Param symName, void* value )
	{
		void** symbol = NULL;
		
		N::FindSymbol( fragment, symName, &symbol );
		
		*symbol = value;
	}
	
	class CFMPluginMain : public MainEntryPoint
	{
		private:
			BinaryImage                        itsBinaryImage;
			NN::Owned< N::CFragConnectionID >  itsFragmentConnection;
		
		public:
			CFMPluginMain( const BinaryImage& image ) : itsBinaryImage( image )
			{
			}
			
			void Invoke( int argc, iota::argv_t argv, iota::environ_t envp );
	};
	
	void CFMPluginMain::Invoke( int argc, iota::argv_t argv, iota::environ_t envp )
	{
		Standard_Entry lamp_main = NULL;
		
		if ( itsFragmentConnection.Get() == N::CFragConnectionID() )
		{
			itsFragmentConnection = ConnectToFragment( itsBinaryImage );
			
			N::FindSymbol( itsFragmentConnection, "\p" "__lamp_main", &lamp_main );
		}
		
		ASSERT( lamp_main != NULL );
		
		lamp_main( argc, argv );
	}
	
	
#if TARGET_RT_MAC_CFM
	
	typedef CFMPluginMain ExternalBinaryMain;
	
#else
	
	typedef CodeResourceMain ExternalBinaryMain;
	
#endif
	
	MainEntry GetMainEntryFromAddress( Trivial_Entry address )
	{
		return MainEntry( new AddressMain( address ) );
	}
	
	MainEntry GetMainEntryFromAddress( Standard_Entry address )
	{
		return MainEntry( new AddressMain( address ) );
	}
	
	MainEntry GetMainEntryFromBinaryImage( const BinaryImage& binary )
	{
		return MainEntry( new ExternalBinaryMain( binary ) );
	}
	
}

