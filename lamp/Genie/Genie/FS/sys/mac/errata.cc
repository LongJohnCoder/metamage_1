/*
	Genie/FS/sys/mac/errata.cc
	--------------------------
*/

#include "Genie/FS/sys/mac/errata.hh"

// Mac OS X
#ifdef __APPLE__
#include <CoreServices/CoreServices.h>
#endif

// Mac OS
#ifndef __CONDITIONALMACROS__
#include <ConditionalMacros.h>
#endif

// Standard C++
#include <algorithm>

// mac-sys-utils
#include "mac_sys/get_machine_name.hh"

// plus
#include "plus/deconstruct_bool.hh"
#include "plus/var_string.hh"

// MacFeatures
#include "MacFeatures/BlueBoxed.hh"

// Genie
#include "Genie/FS/FSTree_Property.hh"
#include "Genie/FS/property.hh"

#if defined( __MACOS__ )  &&  !TARGET_API_MAC_CARBON
	
	#include "Genie/FS/sys/mac/unit.hh"
	
#endif


namespace Genie
{
	
	struct RunningInClassic
	{
		static bool Test()  { return MacFeatures::Is_BlueBoxed(); }
	};
	
	
	struct RunningInRosetta
	{
		static bool Test()
		{
			if ( !TARGET_API_MAC_CARBON )
			{
				// 	Only Carbon apps can run in Rosetta
				return false;
			}
			
			const unsigned char* name = mac::sys::get_machine_name();
			
			const bool powerpc = name != NULL  &&  name[1] == 'P';
			
			return !powerpc;
		}
	};
	
#if !TARGET_API_MAC_CARBON
	
	static bool DriverIsFromSheepShaver( AuxDCEHandle dce )
	{
		const unsigned char* name = GetDriverName_WithinHandle( dce );
		
		return name[0] != 0 && std::equal( name + 1 + (name[0] == '.'),
		                                   name + 1 + name[0],
		                                   "Display_Video_Apple_Sheep" );
	}
	
#endif
	
	struct RunningInSheepShaver
	{
		static bool Test();
	};
	
	bool RunningInSheepShaver::Test()
	{
	#if !TARGET_API_MAC_CARBON
		
		namespace N = Nitrogen;
		
		N::UnitTableDrivers_Container drivers = N::UnitTableDrivers();
		
		N::UnitTableDrivers_Container::const_iterator it = std::find_if( drivers.begin(),
		                                                                 drivers.end(),
		                                                                 std::ptr_fun( DriverIsFromSheepShaver ) );
		
		return it != drivers.end();
		
	#endif
		
		return false;  // FIXME:  What about Carbon?
	}
	
	struct RunningInWeakEmulator
	{
		static bool Test()
		{
			return RunningInSheepShaver::Test() || RunningInRosetta::Test();
		}
	};
	
	
	template < class Erratum >
	struct sys_mac_errata_Property : readonly_property
	{
		static const std::size_t fixed_size = 1;
		
		static void get( plus::var_string& result, const FSTree* that, bool binary )
		{
			plus::deconstruct_bool::apply( result, Erratum::Test(), binary );
		}
	};
	
	#define PROPERTY( prop )  &new_property, &property_params_factory< prop >::value
	
	const vfs::fixed_mapping sys_mac_errata_Mappings[] =
	{
		{ "async-io-race",            PROPERTY( sys_mac_errata_Property< RunningInClassic      > ) },
		{ "fatal-powerpc-exceptions", PROPERTY( sys_mac_errata_Property< RunningInWeakEmulator > ) },
		
		{ NULL, NULL }
	};
	
}

