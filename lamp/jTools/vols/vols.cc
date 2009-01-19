/*	=======
 *	vols.cc
 *	=======
 */

// Standard C++
#include <algorithm>

// Standard C/C++
#include <cerrno>
#include <cstdio>
#include <cstring>

// POSIX
#include <unistd.h>

// Nucleus
#include "Nucleus/Shared.h"

// POSeven
#include "POSeven/Directory.hh"
#include "POSeven/extras/slurp.hh"
#include "POSeven/functions/dirfd.hh"
#include "POSeven/functions/open.hh"
#include "POSeven/functions/openat.hh"
#include "POSeven/functions/opendir.hh"
#include "POSeven/functions/write.hh"

// Orion
#include "Orion/GetOptions.hh"
#include "Orion/Main.hh"


namespace tool
{
	
	namespace NN = Nucleus;
	namespace p7 = poseven;
	namespace O = Orion;
	
	
	int Main( int argc, iota::argv_t argv )
	{
		const char* wanted_driver_name = NULL;
		
		bool ramdisk_only = false;
		
		O::BindOption( "--driver", wanted_driver_name );
		
		O::BindOption( "--ram", ramdisk_only );
		
		O::GetOptions( argc, argv );
		
		if ( ramdisk_only )
		{
			wanted_driver_name = ".EDisk";
		}
		
		const char* vol_path = "/sys/mac/vol";
		
		NN::Shared< p7::dir_t > vol_dir = p7::opendir( vol_path );
		
		typedef p7::directory_contents_container directory_container;
		
		directory_container contents = p7::directory_contents( vol_dir );
		
		typedef directory_container::const_iterator Iter;
		
		for ( Iter it = contents.begin();  it != contents.end();  ++it )
		{
			const char* vol_name = *it;
			
			NN::Owned< p7::fd_t > vol_N_dirfd = p7::openat( p7::dirfd( vol_dir ), vol_name, p7::o_rdonly | p7::o_directory );
			
			NN::Owned< p7::fd_t > name_fd = p7::openat( vol_N_dirfd.get(), "driver/name", p7::o_rdonly | p7::o_binary );
			
			bool wanted = true;
			
			if ( wanted_driver_name != NULL )
			{
				std::string driver_name = p7::slurp( name_fd.get() );
				
				wanted = driver_name == wanted_driver_name;
			}
			
			if ( wanted )
			{
				std::string message = vol_path;
				
				message += '/';
				message += vol_name;
				message += "\n";
				
				p7::write( p7::stdout_fileno, message );
			}
		}
		
		return 0;
	}
	
}

