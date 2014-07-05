/*	===========
 *	Terminal.cc
 *	===========
 */

#include "Genie/IO/Terminal.hh"

// Standard C
#include <signal.h>

// POSIX
#include <fcntl.h>
#include "sys/ttycom.h"

// poseven
#include "poseven/types/errno_t.hh"

// vfs
#include "vfs/node.hh"
#include "vfs/filehandle/methods/filehandle_method_set.hh"
#include "vfs/filehandle/methods/general_method_set.hh"
#include "vfs/filehandle/methods/stream_method_set.hh"
#include "vfs/filehandle/methods/terminal_method_set.hh"
#include "vfs/filehandle/primitives/ioctl.hh"
#include "vfs/filehandle/primitives/poll.hh"
#include "vfs/filehandle/primitives/read.hh"
#include "vfs/filehandle/primitives/write.hh"

// relix
#include "relix/api/current_process.hh"
#include "relix/api/get_process_group.hh"
#include "relix/api/get_process_group_in_session.hh"
#include "relix/signal/signal_process_group.hh"
#include "relix/task/process.hh"
#include "relix/task/process_group.hh"
#include "relix/task/session.hh"


namespace Genie
{
	
	namespace p7 = poseven;
	
	
	static vfs::filehandle& get_tty( vfs::filehandle* that )
	{
		TerminalHandle& terminal = static_cast< TerminalHandle& >( *that );
		
		vfs::filehandle* tty = terminal.Next();
		
		if ( tty == NULL )
		{
			p7::throw_errno( ENXIO );
		}
		
		return *tty;
	}
	
	static unsigned terminal_poll( vfs::filehandle* that )
	{
		return poll( get_tty( that ) );
	}
	
	static ssize_t terminal_read( vfs::filehandle* that, char* buffer, size_t n )
	{
		return read( get_tty( that ), buffer, n );
	}
	
	static ssize_t terminal_write( vfs::filehandle* that, const char* buffer, size_t n )
	{
		return write( get_tty( that ), buffer, n );
	}
	
	static const vfs::stream_method_set terminal_stream_methods =
	{
		&terminal_poll,
		&terminal_read,
		&terminal_write,
	};
	
	static void terminal_ioctl( vfs::filehandle* that, unsigned long request, int* argp )
	{
		static_cast< TerminalHandle& >( *that ).IOCtl( request, argp );
	}
	
	static void terminal_conjoin( vfs::filehandle& that, vfs::filehandle& target )
	{
		static_cast< TerminalHandle& >( that ).Attach( &target );
	}
	
	static void terminal_hangup( vfs::filehandle* that )
	{
		TerminalHandle& terminal = static_cast< TerminalHandle& >( *that );
		
		terminal.Disconnect();
	}
	
	static const vfs::general_method_set terminal_general_methods =
	{
		NULL,
		&terminal_ioctl,
		NULL,
		&terminal_conjoin,
	};
	
	static const vfs::terminal_method_set terminal_methods =
	{
		&terminal_hangup
	};
	
	static const vfs::filehandle_method_set filehandle_methods =
	{
		NULL,
		NULL,
		&terminal_stream_methods,
		&terminal_general_methods,
		&terminal_methods
	};
	
	TerminalHandle::TerminalHandle( const vfs::node& tty_file )
	:
		vfs::filehandle     ( &tty_file, O_RDWR, &filehandle_methods ),
		its_process_group_id( no_pgid  ),
		it_is_disconnected  ( false )
	{
	}
	
	void TerminalHandle::setpgrp( pid_t pgid )
	{
		its_process_group_id = pgid;
		
		if ( it_is_disconnected )
		{
			relix::signal_process_group( SIGHUP, pgid );
		}
	}
	
	static void CheckControllingTerminal( const vfs::filehandle* ctty, const vfs::filehandle& tty )
	{
		if ( ctty != &tty )
		{
			p7::throw_errno( ENOTTY );
		}
	}
	
	static bool session_controls_pgrp( const relix::session& session, pid_t pgid )
	{
		if ( pgid == no_pgid )
		{
			return true;
		}
		
		relix::process_group* pgrp = relix::get_process_group( pgid );
		
		return pgrp  &&  &pgrp->get_session() == &session;
	}
	
	void TerminalHandle::IOCtl( unsigned long request, int* argp )
	{
		relix::process& current = relix::current_process();
		
		relix::process_group& process_group = current.get_process_group();
		
		relix::session& process_session = process_group.get_session();
		
		vfs::filehandle* ctty = process_session.get_ctty().get();
		
		switch ( request )
		{
			case TIOCGPGRP:
				ASSERT( argp != NULL );
				
				CheckControllingTerminal( ctty, *this );
				
				*argp = its_process_group_id;
				
				break;
			
			case TIOCSPGRP:
				ASSERT( argp != NULL );
				
				CheckControllingTerminal( ctty, *this );
				
				{
					// If the terminal has an existing foreground process group,
					// it must be in the same session as the calling process.
					if ( session_controls_pgrp( process_session, its_process_group_id ) )
					{
						// This must be the caller's controlling terminal.
						if ( ctty == this )
						{
							setpgrp( relix::get_process_group_in_session( *argp, process_session )->id() );
						}
					}
					
					p7::throw_errno( ENOTTY );
				}
				
				break;
			
			case TIOCSCTTY:
				if ( process_session.id() != current.id() )
				{
					// not a session leader
					p7::throw_errno( EPERM );
				}
				
				if ( ctty != NULL )
				{
					// already has a controlling terminal
					p7::throw_errno( EPERM );
				}
				
				// Check that we're not the controlling tty of another session
				
				this->setpgrp( process_group.id() );
				
				process_session.set_ctty( *this );
				break;
			
			default:
				if ( its_tty.get() == NULL )
				{
					p7::throw_errno( EINVAL );
				}
				
				ioctl( *its_tty, request, argp );
				
				break;
		};
	}
	
	void TerminalHandle::Disconnect()
	{
		it_is_disconnected = true;
		
		relix::signal_process_group( SIGHUP, its_process_group_id );
	}
	
}

