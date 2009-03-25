/*	===============
 *	PropertyFile.hh
 *	===============
 */

#ifndef GENIE_IO_PROPERTYFILE_HH
#define GENIE_IO_PROPERTYFILE_HH

// Genie
#include "Genie/IO/VirtualFile.hh"


namespace Genie
{
	
	class PropertyReaderFileHandle : public VirtualFileHandle
	{
		private:
			std::string itsData;
		
		public:
			PropertyReaderFileHandle( const FSTreePtr&    file,
			                          OpenFlags           flags,
			                          const std::string&  value )
			:
				VirtualFileHandle( file, flags ),
				itsData( value )
			{
			}
			
			boost::shared_ptr< IOHandle > Clone();
			
			ssize_t SysRead( char* buffer, std::size_t byteCount );
			
			off_t GetEOF() const  { return itsData.size(); }
	};
	
	
	class PropertyWriterFileHandle : public VirtualFileHandle
	{
		private:
			typedef void (*WriteHook)( const FSTree  *that,
			                           const char    *begin,
			                           const char    *end,
			                           bool           binary );
			
			WriteHook  itsWriteHook;
			bool       itIsBinary;
		
		public:
			PropertyWriterFileHandle( const FSTreePtr&  file,
			                          OpenFlags         flags,
			                          WriteHook         writeHook,
			                          bool              binary )
			:
				VirtualFileHandle( file, flags ),
				itsWriteHook( writeHook ),
				itIsBinary( binary )
			{
			}
			
			boost::shared_ptr< IOHandle > Clone();
			
			void SetEOF( off_t length )  {}  // FIXME
			
			ssize_t SysWrite( const char* buffer, std::size_t byteCount );
	};
	
}

#endif

