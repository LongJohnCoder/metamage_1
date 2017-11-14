/*
	pseudotty.cc
	------------
*/

#include "relix/fs/pseudotty.hh"

// gear
#include "gear/inscribe_decimal.hh"

// Debug
#include "debug/boost_assert.hh"

// Boost
#include <boost/intrusive_ptr.hpp>

// plus
#include "plus/conduit.hh"
#include "plus/var_string.hh"

// vfs
#include "vfs/filehandle.hh"
#include "vfs/node.hh"
#include "vfs/enum/poll_result.hh"
#include "vfs/filehandle/functions/nonblocking.hh"
#include "vfs/filehandle/methods/filehandle_method_set.hh"
#include "vfs/filehandle/methods/stream_method_set.hh"
#include "vfs/filehandle/primitives/conjoin.hh"
#include "vfs/filehandle/types/dynamic_group.hh"
#include "vfs/functions/resolve_pathname.hh"

// relix-kernel
#include "relix/api/broken_pipe.hh"
#include "relix/api/root.hh"
#include "relix/api/try_again.hh"
#include "relix/fs/pts_tag.hh"
#include "relix/fs/terminal.hh"


namespace relix
{
	
	typedef std::size_t TerminalID;
	
	struct pseudotty_extra
	{
		unsigned        id;
		plus::conduit*  input;
		plus::conduit*  output;
	};
	
	class PseudoTTYHandle : public vfs::filehandle
	{
		public:
			PseudoTTYHandle( std::size_t                            id,
			                 boost::intrusive_ptr< plus::conduit >  input,
			                 boost::intrusive_ptr< plus::conduit >  output );
	};
	
	static
	unsigned pseudotty_poll( vfs::filehandle* that );
	
	static
	ssize_t pseudotty_read( vfs::filehandle* that, char* buffer, size_t n );
	
	static
	ssize_t pseudotty_write( vfs::filehandle* that, const char* buffer, size_t n );
	
	static const vfs::stream_method_set pseudotty_stream_methods =
	{
		&pseudotty_poll,
		&pseudotty_read,
		&pseudotty_write,
	};
	
	static const vfs::filehandle_method_set pseudotty_methods =
	{
		NULL,
		NULL,
		&pseudotty_stream_methods,
	};
	
	
	static inline vfs::dynamic_group& GetPseudoTTYMap()
	{
		return vfs::get_dynamic_group< pts_tag >();
	}
	
	static inline vfs::filehandle_ptr
	//
	NewPseudoTTY( TerminalID                                    id,
	              const boost::intrusive_ptr< plus::conduit >&  input,
	              const boost::intrusive_ptr< plus::conduit >&  output )
	{
		return new PseudoTTYHandle( id, input, output );
	}
	
	static inline plus::string make_devpts( size_t id )
	{
		plus::var_string result = "/dev/pts/";
		
		result += gear::inscribe_decimal( id );
		
		return result;
	}
	
	void new_pseudotty_pair( vfs::filehandle_ptr&  master,
	                         vfs::filehandle_ptr&  slave )
	{
		static TerminalID index = 0;
		
		boost::intrusive_ptr< plus::conduit > incoming( new plus::conduit );
		boost::intrusive_ptr< plus::conduit > outgoing( new plus::conduit );
		
		vfs::filehandle_ptr master_handle( NewPseudoTTY( index, outgoing, incoming ) );
		vfs::filehandle_ptr slave_handle ( NewPseudoTTY( index, incoming, outgoing ) );
		
		vfs::set_dynamic_element_by_id< pts_tag >( index, slave_handle.get() );
		
		vfs::filehandle_ptr terminal = new_terminal( *vfs::resolve_absolute_path( *root(), make_devpts( index ) ) );
		
		conjoin( *terminal, *slave_handle );
		
		++index;
		
		master.swap( master_handle );
		slave .swap( terminal      );
	}
	
	
	static
	void destroy_pseudotty( vfs::filehandle* that );
	
	PseudoTTYHandle::PseudoTTYHandle( std::size_t                            id,
			                          boost::intrusive_ptr< plus::conduit >  input,
			                          boost::intrusive_ptr< plus::conduit >  output )
	: vfs::filehandle( O_RDWR, &pseudotty_methods, sizeof (pseudotty_extra), &destroy_pseudotty )
	{
		pseudotty_extra& extra = *(pseudotty_extra*) this->extra();
		
		extra.id     = id;
		extra.input  = input.get();
		extra.output = output.get();
		
		intrusive_ptr_add_ref( extra.input  );
		intrusive_ptr_add_ref( extra.output );
	}
	
	static
	void destroy_pseudotty( vfs::filehandle* that )
	{
		pseudotty_extra& extra = *(pseudotty_extra*) that->extra();
		
		extra.input->close_egress();
		extra.output->close_ingress();
		
		GetPseudoTTYMap().erase( extra.id );
		
		intrusive_ptr_release( extra.input  );
		intrusive_ptr_release( extra.output );
	}
	
	static
	unsigned pseudotty_poll( vfs::filehandle* that )
	{
		pseudotty_extra& extra = *(pseudotty_extra*) that->extra();
		
		return (extra.input->is_readable() ? vfs::Poll_read : 0) | vfs::Poll_write;
	}
	
	static
	ssize_t pseudotty_read( vfs::filehandle* that, char* buffer, size_t n )
	{
		pseudotty_extra& extra = *(pseudotty_extra*) that->extra();
		
		return extra.input->read( buffer, n, is_nonblocking( *that ), &try_again );
	}
	
	static
	ssize_t pseudotty_write( vfs::filehandle* that, const char* buffer, size_t n )
	{
		pseudotty_extra& extra = *(pseudotty_extra*) that->extra();
		
		return extra.output->write( buffer, n, is_nonblocking( *that ), &try_again, &broken_pipe );
	}
	
}
