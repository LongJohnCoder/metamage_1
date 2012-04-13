/*	==============
 *	RegularFile.hh
 *	==============
 */

#ifndef GENIE_IO_REGULARFILE_HH
#define GENIE_IO_REGULARFILE_HH

 // Genie
 #include "Genie/IO/File.hh"


namespace Genie
{
	
	class RegularFileHandle : public FileHandle
	{
		private:
			off_t itsMark;
		
		public:
			RegularFileHandle( int flags );
			
			virtual ~RegularFileHandle();
			
			bool IsRegularFile() const  { return true; }
			
			virtual IOPtr Clone() = 0;
			
			virtual unsigned int SysPoll()  { return kPollRead | kPollWrite; }
			
			virtual ssize_t Positioned_Read( char* buffer, size_t n_bytes, off_t offset );
			
			virtual ssize_t Positioned_Write( const char* buffer, size_t n_bytes, off_t offset );
			
			virtual ssize_t Append( const char* buffer, size_t n_bytes );
			
			ssize_t SysRead( char* buffer, size_t n_bytes );
			
			ssize_t SysWrite( const char* buffer, size_t n_bytes );
			
			off_t Seek( off_t offset, int whence );
			
			virtual off_t GetEOF();
			
			virtual void SetEOF( off_t length );
			
			off_t GetFileMark() const  { return itsMark; }
			
			ssize_t Advance( ssize_t step )  { itsMark += step;  return step; }
			
			ssize_t Write( const char* buffer, std::size_t byteCount );
			
			boost::intrusive_ptr< memory_mapping > Map( size_t length, off_t offset );
	};
	
	template <> struct IOHandle_Downcast_Traits< RegularFileHandle >
	{
		static IOHandle::Test GetTest()  { return &IOHandle::IsRegularFile; }
		
		static int GetError( IOHandle& handle )
		{
			return IOHandle_Downcast_Traits< StreamHandle >::GetError( handle );
		}
	};
	
}

#endif

