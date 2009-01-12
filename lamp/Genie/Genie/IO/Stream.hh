/*	=========
 *	Stream.hh
 *	=========
 */

#ifndef GENIE_IO_STREAM_HH
#define GENIE_IO_STREAM_HH

// Standard C++
#include <string>

// POSIX
#include "errno.h"
#include <fcntl.h>

// Genie
#include "Genie/IO/Base.hh"


namespace Genie
{
	
	class StreamHandle : public IOHandle
	{
		private:
			OpenFlags    itsOpenFlags;
			std::string  itsPeekBuffer;
		
		public:
			StreamHandle( OpenFlags flags );
			
			virtual ~StreamHandle();
			
			bool IsStream() const  { return true; }
			
			OpenFlags GetFlags() const  { return itsOpenFlags; }
			
			void SetFlags( OpenFlags flags )  { itsOpenFlags = flags; }
			
			virtual unsigned int SysPoll() = 0;
			
			virtual ssize_t SysRead( char* data, std::size_t byteCount );
			
			virtual ssize_t SysWrite( const char* data, std::size_t byteCount );
			
			virtual void IOCtl( unsigned long request, int* argp );
			
			virtual bool IsDisconnected() const  { return false; }
			
			bool IsNonblocking() const  { return itsOpenFlags & O_NONBLOCK; }
			
			void SetNonblocking  ()  { itsOpenFlags |=  O_NONBLOCK; }
			void ClearNonblocking()  { itsOpenFlags &= ~O_NONBLOCK; }
			
			void TryAgainLater() const;
			
			const std::string* Peek( ByteCount minBytes );
			
			unsigned int Poll();
			
			ssize_t Read( char* data, std::size_t byteCount );
			
			virtual ssize_t Write( const char* data, std::size_t byteCount );
	};
	
	template <> struct IOHandle_Downcast_Traits< StreamHandle >
	{
		static IOHandle::Test GetTest()  { return &IOHandle::IsStream; }
		
		static int GetError( IOHandle& handle )  { return handle.IsDirectory() ? EISDIR : EINVAL; }
	};
	
}

#endif

