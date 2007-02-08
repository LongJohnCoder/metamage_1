/*	=======
 *	SMTP.cc
 *	=======
 */

#include "SMTP.hh"

#include <algorithm>
#include <string>


namespace SMTP
{
	
	namespace Client
	{
		
		ResponseCode::ResponseCode( const std::string& response )
		{
			if ( response.size() < size )
			{
				throw InvalidResponse();
			}
			
			std::copy( response.begin(), response.begin() + size, chars );
		}
		
		bool CheckResponse( const std::string& response )
		{
			if ( response.size() >= ResponseCode::size )
			{
				switch ( response[ 3 ] )
				{
					case ' ':  return true;
					case '-':  return false;
					
					default:  break;
				}
			}
			
			throw InvalidResponse();
		}
		
		ResponseCode VerifySuccess( ResponseCode code )
		{
			if ( code.chars[ 0 ] == '4' )
			{
				throw Error();
			}
			
			if ( code.chars[ 0 ] == '5' )
			{
				throw Failure();
			}
			
			return code;
		}
		
	}  // namespace Client
	
}  // namespace SMTP

