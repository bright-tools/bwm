#include "MouseHistory.hpp"

bool MouseHistory::InTime( DWORD p_time )
{
	return ( ( p_time - time ) <= GetDoubleClickTime() );
}

bool MouseHistory::InBounds( POINT pt )
{
	const int wd = GetSystemMetrics( SM_CXDOUBLECLK ) / 2;
	const int ht = GetSystemMetrics( SM_CYDOUBLECLK ) / 2;
	return ( ( ( pt.x >= ( position.x - wd ) ) && ( pt.x < ( position.x + wd ) ) ) && ( ( ( position.y >= ( pt.y - ht ) ) && ( position.y <= ( pt.y + ht ) ) ) ) );
}

MouseHistory::MouseHistory()
{
	button = M_NONE;
}

bool MouseHistory::IsDoubleClick( const enum MouseButton p_button, const MOUSEHOOKSTRUCT p_ms )
{
	const DWORD nowTime = GetTickCount();
	// perform check
	bool isDblClk = ( ( button == p_button ) && ( InTime( nowTime ) ) && ( InBounds( p_ms.pt ) ) );

	// update history
	button = p_button;
	time = nowTime;
	position = p_ms.pt;

	if( isDblClk )
	{
		button = M_NONE;
	}
	// return result
	return isDblClk;
}
