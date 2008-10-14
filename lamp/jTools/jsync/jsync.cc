/*	========
 *	jsync.cc
 *	========
 */

// Standard C++
#include <functional>
#include <vector>

// Standard C/C++
#include <cstdio>
#include <cstdlib>

// POSIX
#include <errno.h>

// Iota
#include "iota/strings.hh"

// MoreFunctional
#include "PointerToFunction.hh"

// Io
#include "io/walk.hh"

// POSeven
#include "POSeven/Directory.hh"
#include "POSeven/Open.hh"
#include "POSeven/Pathnames.hh"
#include "POSeven/extras/pump.hh"
#include "POSeven/functions/fchmod.hh"
#include "POSeven/functions/fstat.hh"
#include "POSeven/functions/mkdir.hh"
#include "POSeven/functions/lseek.hh"
#include "POSeven/functions/stat.hh"
#include "POSeven/functions/utime.hh"
#include "POSeven/types/exit_t.hh"

// Orion
#include "Orion/GetOptions.hh"
#include "Orion/Main.hh"


namespace tool
{
	
	namespace NN = Nucleus;
	namespace p7 = poseven;
	namespace O = Orion;
	
	
	using namespace io::path_descent_operators;
	
	
	static bool globally_verbose = false;
	static bool global_dry_run = false;
	
	static bool globally_locking_files = false;
	
	
	static const std::string& mkdir_path( const std::string& path )
	{
		if ( !io::directory_exists( path ) )
		{
			mkdir_path( io::get_preceding_directory( path ) );
			
			p7::mkdir( path );
		}
		
		return path;
	}
	
	
	static bool filter_file( const std::string& path )
	{
		std::string filename = io::get_filename( path );
		
		return filename == "Icon\r"
		    || filename == ".DS_Store";
	}
	
	static bool filter_directory( const std::string& path )
	{
		std::string filename = io::get_filename( path );
		
		return filename == "CVS"
		    || filename == "CVSROOT";
	}
	
	static bool filter_item( const std::string& path )
	{
		return filter_file( path ) || filter_directory( path );
	}
	
	
	inline void copy_modification_date( p7::fd_t in, const std::string& out_file )
	{
		// Copy the modification date
		p7::utime( out_file, p7::fstat( in ).st_mtime );
	}
	
	static void copy_file( const std::string& source, const std::string& dest )
	{
		//p7::copyfile( source, dest );
		
		// Lock backup files to prevent accidents
		p7::mode_t mode = p7::mode_t( globally_locking_files ? 0400 : 0600 );
		
		NN::Owned< p7::fd_t > in  = p7::open( source, p7::o_rdonly );
		NN::Owned< p7::fd_t > out = p7::open( dest,   p7::o_wronly | p7::o_creat | p7::o_excl, mode );
		
		p7::pump( in, out );
		
		p7::close( out );
		
		copy_modification_date( in, dest );
	}
	
	static void recursively_copy_directory( const std::string& source, const std::string& dest );
	
	static void recursively_copy( const std::string& source, const std::string& dest )
	{
		if ( io::file_exists( source ) )
		{
			if ( !filter_file( source ) )
			{
				copy_file( source, dest );
			}
		}
		else
		{
			recursively_copy_directory( source, dest );
		}
	}
	
	static void recursively_copy_into( const std::string& source, const std::string& dest )
	{
		recursively_copy( source, dest / io::get_filename( source ) );
	}
	
	static void recursively_copy_directory_contents( const std::string& source, const std::string& dest )
	{
		typedef p7::directory_contents_container directory_container;
		
		directory_container contents = io::directory_contents( source );
		
		std::for_each( contents.begin(),
		               contents.end(),
		               std::bind2nd( more::ptr_fun( recursively_copy_into ),
		                             dest ) );
	}
	
	static void recursively_copy_directory( const std::string& source, const std::string& dest )
	{
		if ( filter_directory( source ) )
		{
			return;
		}
		
		p7::mkdir( dest );
		
		recursively_copy_directory_contents( source, dest );
	}
	
	
	static void compare_3_files( p7::fd_t  a,
	                             p7::fd_t  b,
	                             p7::fd_t  c,
	                             bool&     a_matches_b,
	                             bool&     b_matches_c,
	                             bool&     c_matches_a )
	{
		const std::size_t buffer_size = 4096;
		
		char a_buffer[ buffer_size ];
		char b_buffer[ buffer_size ];
		char c_buffer[ buffer_size ];
		
		while ( a_matches_b || b_matches_c || c_matches_a )
		{
			ssize_t a_read = 0;
			ssize_t b_read = 0;
			ssize_t c_read = 0;
			
			if ( c_matches_a || a_matches_b )
			{
				a_read = p7::read( a, a_buffer, buffer_size );
			}
			
			if ( a_matches_b || b_matches_c )
			{
				b_read = p7::read( b, b_buffer, buffer_size );
			}
			
			if ( b_matches_c || c_matches_a )
			{
				c_read = p7::read( c, c_buffer, buffer_size );
			}
			
			if ( a_read + b_read + c_read == 0 )
			{
				break;
			}
			
			if ( a_matches_b )
			{
				a_matches_b = a_read == b_read  &&  std::equal( a_buffer, a_buffer + a_read, b_buffer );
			}
			
			if ( b_matches_c )
			{
				b_matches_c = c_read == b_read  &&  std::equal( c_buffer, c_buffer + c_read, b_buffer );
			}
			
			if ( c_matches_a )
			{
				c_matches_a = a_read == c_read  &&  std::equal( a_buffer, a_buffer + a_read, c_buffer );
			}
		}
	}
	
	
	static void sync_files( const std::string&  a,
	                        const std::string&  b,
	                        const std::string&  c,
	                        bool                b_exists )
	{
		if ( globally_verbose )
		{
			//std::printf( "%s\n", a.c_str() );
		}
		
		NN::Owned< p7::fd_t > a_fd = p7::open( a, p7::o_rdonly );
		NN::Owned< p7::fd_t > c_fd = p7::open( c, p7::o_rdonly );
		
		NN::Owned< p7::fd_t > b_fd;
		
		if ( b_exists )
		{
			b_fd = p7::open( b, p7::o_rdonly );
			
			time_t a_time = p7::fstat( a_fd ).st_mtime;
			time_t b_time = p7::fstat( b_fd ).st_mtime;
			time_t c_time = p7::fstat( c_fd ).st_mtime;
			
			if ( a_time == b_time  &&  c_time == b_time )
			{
				return;
			}
		}
		
		bool a_matches_b = b_exists;
		bool b_matches_c = b_exists;
		bool c_matches_a = true;
		
		compare_3_files( a_fd,
		                 b_fd,
		                 c_fd,
		                 a_matches_b,
		                 b_matches_c,
		                 c_matches_a );
		
		if ( a_matches_b && b_matches_c )
		{
			return;
		}
		
		if ( !c_matches_a )
		{
			// A and C are different from each other,
			// so at least one must have changed from B
			
			if ( !a_matches_b && !b_matches_c )
			{
				const char* status = b_exists ? "requires 3-way merge"
				                              : "added simultaneously with different contents";
				
				std::printf( "%s %s\n", a.c_str(), status );
				
				return;
			}
			
			// At least one of A and C matched B, and it can't be both.
			// A copy is required.
			
			std::printf( "%s %s\n", a_matches_b ? "Incoming"
			                                    : "Outgoing", a.c_str() );
			
			if ( global_dry_run )
			{
				return;
			}
			
			if ( b_matches_c )
			{
				// copy a to c
				p7::lseek( a_fd, 0 );
				
				p7::close( c_fd );
				
				c_fd = p7::open( c, p7::o_rdwr | p7::o_trunc );
				
				p7::pump( a_fd, c_fd );
				
				copy_modification_date( a_fd, c );
			}
			else
			{
				// copy c to a
				p7::lseek( c_fd, 0 );
				
				p7::close( a_fd );
				
				a_fd = p7::open( a, p7::o_rdwr | p7::o_trunc );
				
				p7::pump( c_fd, a_fd );
				
				copy_modification_date( c_fd, a );
			}
		}
		else
		{
			std::printf( "%s %s\n", b_exists ? "Updating" : "Adding", a.c_str() );
		}
		
		if ( global_dry_run )
		{
			return;
		}
		
		// A and C match
		
		// copy a to b
		p7::lseek( a_fd, 0 );
		
		if ( b_exists )
		{
			p7::fchmod( b_fd, p7::mode_t( 0600 ) );  // unlock
			
			p7::close( b_fd );
		}
		
		b_fd = p7::open( b, p7::o_rdwr | p7::o_trunc | p7::o_creat, p7::mode_t( 0400 ) );
		
		p7::pump( a_fd, b_fd );
		
		copy_modification_date( a_fd, b );
		
		if ( b_exists )
		{
			p7::fchmod( b_fd, p7::mode_t( 0400 ) );  // lock
		}
	}
	
	static void recursively_sync_directories( const std::string&  a,
	                                          const std::string&  b,
	                                          const std::string&  c );
	
	static void recursively_sync( const std::string&  a,
	                              const std::string&  b,
	                              const std::string&  c )
	{
		const bool b_exists = io::item_exists( b );
		
		bool a_is_dir = io::directory_exists( a );
		bool b_is_dir = io::directory_exists( b );
		bool c_is_dir = io::directory_exists( c );
		
		if ( bool matched = a_is_dir == c_is_dir  &&  (!b_exists || a_is_dir == b_is_dir) )
		{
			if ( a_is_dir )
			{
				if ( !b_exists )
				{
					p7::mkdir( b );
				}
				
				recursively_sync_directories( a, b, c );
			}
			else
			{
				sync_files( a, b, c, b_exists );
			}
		}
		else if ( b_exists )
		{
			// file vs. directory
			if ( a_is_dir != b_is_dir )
			{
				std::printf( "%s changed from %s to %s\n",
				              a.c_str(),      b_is_dir ? "directory" : "file",
				                                    a_is_dir ? "directory" : "file" );
			}
			
			if ( c_is_dir != b_is_dir )
			{
				std::printf( "%s changed from %s to %s\n",
				              c.c_str(),      b_is_dir ? "directory" : "file",
				                                    c_is_dir ? "directory" : "file" );
			}
		}
		else
		{
			std::printf( "Add conflict in %s (file vs. directory)\n", a.c_str() );
		}
	}
	
	static void odd_item( const std::string& path, bool new_vs_old )
	{
		std::printf( "%s is %s\n", path.c_str(), new_vs_old ? "new" : "old" );
	}
	
	inline void new_item( const std::string& path )
	{
		odd_item( path, true );
	}
	
	inline void old_item( const std::string& path )
	{
		odd_item( path, false );
	}
	
	template < class In, class Out, class Pred >
	Out copy_if( In begin, In end, Out result, Pred f )
	{
		while ( begin != end )
		{
			if ( f( *begin ) )
			{
				*result++ = *begin;
			}
			
			++begin;
		}
		
		return result;
	}
	
	template < class In, class Out, class Pred >
	Out copy_unless( In begin, In end, Out result, Pred f )
	{
		while ( begin != end )
		{
			if ( !f( *begin ) )
			{
				*result++ = *begin;
			}
			
			++begin;
		}
		
		return result;
	}
	
	inline const std::string& identity( const std::string& s )
	{
		return s;
	}
	
	struct null_iterator
	{
		template < class Data >  void operator=( Data )  {}
		
		null_iterator operator++(     )  { return null_iterator(); }
		null_iterator operator++( int )  { return null_iterator(); }
		
		null_iterator operator*()  { return null_iterator(); }
	};
	
	template < class Type >
	static int compare( const Type& a, const Type& b )
	{
		return   a < b ? -1
		       : a > b ?  1
		       :          0;
	}
	
	/*
	inline int compare( const std::string& a, const std::string& b )
	{
		return std::lexicographical_compare_3way( a.begin(), a.end(), b.begin(), b.end() );
	}
	*/
	
	template < class Iter, class F, class A, class B, class Both >
	static void compare_sequences( Iter a_begin, Iter a_end,
	                               Iter b_begin, Iter b_end, F f, A a_only,
	                                                              B b_only, Both both )
	{
		while ( a_begin != a_end  &&  b_begin != b_end )
		{
			typename F::result_type a_result = f( *a_begin );
			typename F::result_type b_result = f( *b_begin );
			
			switch ( compare( a_result, b_result ) )
			{
				case 0:
					*both++ = a_result;
					
					++a_begin;
					++b_begin;
					
					break;
				
				case -1:
					*a_only++ = a_result;
					
					a_begin++;
					break;
				
				case 1:
					*b_only++ = b_result;
					
					b_begin++;
					break;
				
			}
		}
		
		std::transform( a_begin, a_end, a_only, f );
		std::transform( b_begin, b_end, b_only, f );
	}
	
	static void recursively_sync_directory_contents( const std::string&  a_dir,
	                                                 const std::string&  b_dir,
	                                                 const std::string&  c_dir )
	{
		typedef p7::directory_contents_container directory_container;
		
		directory_container a_contents = io::directory_contents( a_dir );
		directory_container b_contents = io::directory_contents( b_dir );
		directory_container c_contents = io::directory_contents( c_dir );
		
		std::vector< std::string > a;
		std::vector< std::string > b;
		std::vector< std::string > c;
		
		copy_unless( a_contents.begin(), a_contents.end(), std::back_inserter( a ), std::ptr_fun( filter_item ) );
		copy_unless( b_contents.begin(), b_contents.end(), std::back_inserter( b ), std::ptr_fun( filter_item ) );
		copy_unless( c_contents.begin(), c_contents.end(), std::back_inserter( c ), std::ptr_fun( filter_item ) );
		
		std::sort( a.begin(), a.end() );
		std::sort( b.begin(), b.end() );
		std::sort( c.begin(), c.end() );
		
		std::vector< std::string > a_added;
		std::vector< std::string > a_removed;
		std::vector< std::string > a_static;
		
		std::vector< std::string > c_added;
		std::vector< std::string > c_removed;
		std::vector< std::string > c_static;
		
		compare_sequences( a.begin(), a.end(),
		                   b.begin(), b.end(),
		                   std::ptr_fun( static_cast< std::string (*)( const std::string& ) >( io::get_filename ) ),
		                   std::back_inserter( a_added   ),
		                   std::back_inserter( a_removed ),
		                   std::back_inserter( a_static  ) );
		
		compare_sequences( c.begin(), c.end(),
		                   b.begin(), b.end(),
		                   std::ptr_fun( static_cast< std::string (*)( const std::string& ) >( io::get_filename ) ),
		                   std::back_inserter( c_added   ),
		                   std::back_inserter( c_removed ),
		                   std::back_inserter( c_static  ) );
		
		std::vector< std::string > a_created;
		std::vector< std::string > c_created;
		std::vector< std::string > mutually_added;
		
		std::vector< std::string > a_deleted;
		std::vector< std::string > c_deleted;
		std::vector< std::string > mutually_deleted;
		
		std::vector< std::string > mutually_static;
		
		compare_sequences( a_added.begin(), a_added.end(),
		                   c_added.begin(), c_added.end(),
		                   std::ptr_fun( identity ),
		                   std::back_inserter( a_created ),
		                   std::back_inserter( c_created ),
		                   std::back_inserter( mutually_added ) );
		
		compare_sequences( a_removed.begin(), a_removed.end(),
		                   c_removed.begin(), c_removed.end(),
		                   std::ptr_fun( identity ),
		                   std::back_inserter( a_deleted ),
		                   std::back_inserter( c_deleted ),
		                   std::back_inserter( mutually_deleted ) );
		
		compare_sequences( a_static.begin(), a_static.end(),
		                   c_static.begin(), c_static.end(),
		                   std::ptr_fun( identity ),
		                   null_iterator(),
		                   null_iterator(),
		                   std::back_inserter( mutually_static ) );
		
		typedef std::vector< std::string >::const_iterator Iter;
		
		for ( Iter it = a_created.begin();  it != a_created.end();  ++ it )
		{
			const std::string& filename = *it;
			
			std::string a_path = a_dir / filename;
			std::string b_path = b_dir / filename;
			std::string c_path = c_dir / filename;
			
			std::printf( "%s created\n", a_path.c_str() );
			
			if ( !global_dry_run )
			{
				globally_locking_files = false;
				
				recursively_copy( a_path, c_path );
				
				globally_locking_files = true;
				
				recursively_copy( a_path, b_path );
			}
		}
		
		for ( Iter it = c_created.begin();  it != c_created.end();  ++ it )
		{
			const std::string& filename = *it;
			
			std::string a_path = a_dir / filename;
			std::string b_path = b_dir / filename;
			std::string c_path = c_dir / filename;
			
			std::printf( "%s created\n", c_path.c_str() );
			
			if ( !global_dry_run )
			{
				globally_locking_files = false;
				
				recursively_copy( c_path, a_path );
				
				globally_locking_files = true;
				
				recursively_copy( c_path, b_path );
			}
		}
		
		for ( Iter it = mutually_added.begin();  it != mutually_added.end();  ++ it )
		{
			const std::string& filename = *it;
			
			std::string a_path = a_dir / filename;
			std::string b_path = b_dir / filename;
			std::string c_path = c_dir / filename;
			
			std::printf( "%s mutually added\n", a_path.c_str() );
			
			recursively_sync( a_path, b_path, c_path );
		}
		
		for ( Iter it = a_deleted.begin();  it != a_deleted.end();  ++ it )
		{
			const std::string& filename = *it;
			
			std::string a_path = a_dir / filename;
			std::string b_path = b_dir / filename;
			std::string c_path = c_dir / filename;
			
			std::printf( "%s deleted\n", a_path.c_str() );
		}
		
		for ( Iter it = c_deleted.begin();  it != c_deleted.end();  ++ it )
		{
			const std::string& filename = *it;
			
			std::string a_path = a_dir / filename;
			std::string b_path = b_dir / filename;
			std::string c_path = c_dir / filename;
			
			std::printf( "%s deleted\n", c_path.c_str() );
		}
		
		for ( Iter it = mutually_deleted.begin();  it != mutually_deleted.end();  ++ it )
		{
			const std::string& filename = *it;
			
			std::string a_path = a_dir / filename;
			std::string b_path = b_dir / filename;
			std::string c_path = c_dir / filename;
			
			std::printf( "%s mutually deleted\n", a_path.c_str() );
			
			if ( !global_dry_run )
			{
				io::recursively_delete( b_path );
			}
		}
		
		for ( Iter it = mutually_static.begin();  it != mutually_static.end();  ++ it )
		{
			const std::string& filename = *it;
			
			std::string a_path = a_dir / filename;
			std::string b_path = b_dir / filename;
			std::string c_path = c_dir / filename;
			
			recursively_sync( a_path, b_path, c_path );
		}
		
		// added/nil:  simple add -- just do it
		// added/added:  mutual add -- check for conflicts
		// static/static:  check for conflicts
		// deleted/static:  single delete -- check for conflicts
		// deleted/deleted:  mutual delete -- just do it
	}
	
	static void recursively_sync_directories( const std::string&  a,
	                                          const std::string&  b,
	                                          const std::string&  c )
	{
		// compare any relevant metadata, like Desktop comment
		
		if ( globally_verbose )
		{
			std::printf( "%s\n", a.c_str() );
		}
		
		recursively_sync_directory_contents( a, b, c );
	}
	
	
	static std::string home_dir_pathname()
	{
		if ( const char* home = std::getenv( "HOME" ) )
		{
			return home;
		}
		
		return "/";
	}
	
	static std::string get_jsync_root_pathname()
	{
		std::string home = home_dir_pathname();
		
		const char* jsync = "Library/JSync";
		
		return mkdir_path( home / jsync );
	}
	
	
	int Main( int argc, iota::argv_t argv )
	{
		O::BindOption( "-v", globally_verbose );
		O::BindOption( "-n", global_dry_run   );
		
		O::GetOptions( argc, argv );
		
		char const *const *free_args = O::FreeArguments();
		
		std::size_t n_args = O::FreeArgumentCount();
		
		const char* default_path = "Default";
		
		const char* path = n_args >= 1 ? free_args[0] : default_path;
		
		std::string jsync_root = get_jsync_root_pathname();
		
		std::string jsync_path = jsync_root / path;
		
		jsync_path += ".jsync";
		
		if ( !io::directory_exists( jsync_path ) )
		{
			std::fprintf( stderr, "jsync: no such sync path '%s'\n"
			                      "(No such directory %s)\n",path,
			                      jsync_path.c_str() );
			
			return 1;
		}
		
		std::string jsync_sandbox = jsync_path / "Sandbox";  // should be a link
		std::string jsync_remote  = jsync_path / "Remote";   // should be a link
		std::string jsync_base    = jsync_path / "Base";
		
		//if ( comparing )
		{
			recursively_sync_directories( jsync_sandbox, jsync_base, jsync_remote );
		}
		
		return 0;
	}

}

