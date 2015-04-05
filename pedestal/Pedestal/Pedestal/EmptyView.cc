/*	============
 *	EmptyView.cc
 *	============
 */

#include "Pedestal/EmptyView.hh"


namespace Pedestal
{
	
	const boost::intrusive_ptr< View >& EmptyView::Get()
	{
		static boost::intrusive_ptr< View > empty = new EmptyView;
		
		return empty;
	}
	
}
