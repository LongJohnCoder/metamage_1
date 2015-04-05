/*
	SetData.cc
	----------
*/

#include "AEObjectModel/SetData.hh"

// Nitrogen
#include "Mac/Toolbox/Utilities/ThrowOSStatus.hh"


namespace Nitrogen
{
	
	void SetData( const Mac::AEDesc_Token& obj, const Mac::AEDesc_Data& data )
	{
		return TheGlobalDataSetter().SetData( obj, data );
	}
	
	DataSetter::DataSetter()
	{
	}
	
	void DataSetter::SetData( const Mac::AEDesc_Token& obj, const Mac::AEDesc_Data& data )
	{
		Map::const_iterator found = map.find( Mac::DescType( obj.descriptorType ) );
		
		if ( found == map.end() )
		{
			Mac::ThrowOSStatus( errAEEventNotHandled );
		}
		
		return found->second( obj, data );
	}
	
	DataSetter& TheGlobalDataSetter()
	{
		static DataSetter theGlobalDataSetter;
		
		return theGlobalDataSetter;
	}
	
}
