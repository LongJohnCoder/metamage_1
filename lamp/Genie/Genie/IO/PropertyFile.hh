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
			
			ssize_t SysRead( char* buffer, std::size_t byteCount );
			
			off_t GetEOF() const  { return itsData.size(); }
	};
	
	
	class PropertyWriterFileHandle : public VirtualFileHandle
	{
		private:
			typedef unsigned long OpaqueKey;
			
			typedef OpaqueKey (*KeyHook)( const FSTree* that );
			
			typedef void (*WriteHook)( OpaqueKey    key,
			                           const char  *begin,
			                           const char  *end );
			
			KeyHook    itsKeyHook;
			WriteHook  itsWriteHook;
		
		public:
			PropertyWriterFileHandle( const FSTreePtr&  file,
			                          OpenFlags         flags,
			                          KeyHook           keyHook,
			                          WriteHook         writeHook )
			:
				VirtualFileHandle( file, flags ),
				itsKeyHook  ( keyHook   ),
				itsWriteHook( writeHook )
			{
			}
			
			void SetEOF( off_t length )  {}  // FIXME
			
			ssize_t SysWrite( const char* buffer, std::size_t byteCount );
	};
	
}

#endif

