/*	==============
 *	VirtualFile.hh
 *	==============
 */

#ifndef GENIE_IO_VIRTUALFILE_HH
#define GENIE_IO_VIRTUALFILE_HH

// Genie
#include "Genie/IO/Base.hh"


namespace Genie
{
	
	template < class Base >
	class VirtualFileHandle : public Base
	{
		private:
			FSTreePtr itsFile;
		
		public:
			VirtualFileHandle( const FSTreePtr&  file,
			                   int               flags ) : Base( flags ),
			                                               itsFile( file )
			{
			}
			
			~VirtualFileHandle();
			
			FSTreePtr GetFile()  { return itsFile; }
	};
	
	template < class Base >
	VirtualFileHandle< Base >::~VirtualFileHandle()
	{
	}
	
}

#endif

