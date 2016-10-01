#include "WinUtils.hpp"
#include "wmmousedll.h"
#include <Windows.h>
#include <Dwmapi.h>

bool IgnoreWindow()
{
	// don't move or resize windows that are maximized
	WINDOWPLACEMENT wndpl;
	wndpl.length = sizeof( WINDOWPLACEMENT );
	::GetWindowPlacement( hWnd, &wndpl );
	if( wndpl.showCmd == SW_SHOWMAXIMIZED )
	{
		return true;
	}

	// add other filters here...

	return false;
}

/* Thanks to http://stackoverflow.com/questions/7277366/why-does-enumwindows-return-more-windows-than-i-expected */
BOOL IsAltTabWindow( HWND hwnd )
{
	TITLEBARINFO ti;
	HWND hwndTry, hwndWalk = NULL;

	if( !IsWindowVisible( hwnd ) )
		return FALSE;

	hwndTry = GetAncestor( hwnd, GA_ROOTOWNER );
	while( hwndTry != hwndWalk )
	{
		hwndWalk = hwndTry;
		hwndTry = GetLastActivePopup( hwndWalk );
		if( IsWindowVisible( hwndTry ) )
			break;
	}
	if( hwndWalk != hwnd )
		return FALSE;

	// the following removes some task tray programs and "Program Manager"
	ti.cbSize = sizeof( ti );
	GetTitleBarInfo( hwnd, &ti );
	if( ti.rgstate[ 0 ] & STATE_SYSTEM_INVISIBLE )
		return FALSE;

	// Tool windows should not be displayed either, these do not appear in the
	// task bar.
	if( GetWindowLong( hwnd, GWL_EXSTYLE ) & WS_EX_TOOLWINDOW )
		return FALSE;

	return TRUE;
}

RECT CalcWindowBorder( HWND p_hWnd )
{
	RECT rect, frame, border;
	GetWindowRect( p_hWnd, &rect );
	DwmGetWindowAttribute( p_hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &frame, sizeof( RECT ) );

	border.left = frame.left - rect.left;
	border.top = frame.top - rect.top;
	border.right = rect.right - frame.right;
	border.bottom = rect.bottom - frame.bottom;

	return border;
}

HWND GetTopLevelWindow( HWND p_hWnd )
{
	// walk up the window hierarchie to find the corresponding
	// top level window. commment this block and enjoy resizing
	// all sorts of widgets :)
	while( p_hWnd != 0 )
	{
		style = ::GetWindowLong( p_hWnd, GWL_STYLE );
		const int x = style & ( WS_POPUP | WS_CHILD );

		if( ( x == WS_OVERLAPPED ) ||
			( x == WS_POPUP )
			) break;

		// we also want to manipulate mdi childs that
		// aren't maximized
		if(
			( !( style & WS_MAXIMIZE ) ) &&
			( ::GetWindowLong( p_hWnd, GWL_EXSTYLE ) & WS_EX_MDICHILD )
			) break;

		p_hWnd = ::GetParent( p_hWnd );
	}
	return p_hWnd;
}