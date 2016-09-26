#include "wmmousedll.h"
#include <Dwmapi.h>
#include <windows.h>

#define MAX_NUM_MODIFIER_KEYS 10U
#define MAX_SNAPS 200U

// enum possible states the window manager is in
enum OpMode
{
    O_NONE,    // we're currently doing nothing
    O_MOVE,    // we're dragging a window
    O_SIZE    // we're sizing a window
};

typedef struct
{
	RECT rect;
	LONG snap;
} snap_t;

typedef struct
{
	snap_t snap_list[ MAX_SNAPS ];
	unsigned size;
	unsigned next;
} snap_info_t;

#define DEFAULT_SNAP_ALLOC 200
#define SNAP_ALLOC_GROW    10

#define    DLL_EXPORT extern "C" __declspec(dllexport)

// Shared DATA
#pragma data_seg ( "SHAREDDATA" )
    // this is the total number of processes this dll is currently attached to
    int                iInstanceCount        = 0;

    // the initial position in which a mouse button is pressed
    POINT            ptMouseInit    = {0,0};

    // the window rect, handle and style beneath the mouse
    RECT            rWndInit    = {0,0,0,0};
    HWND            hWnd        = 0;
    int                style        = 0;
	// Size of the window border
	RECT border;

    // the current mode of operation
    OpMode            opMode;

	snap_info_t x_snaps = { NULL, MAX_SNAPS, 0 };
	snap_info_t y_snaps = { NULL, MAX_SNAPS, 0 };

	unsigned MonitorSnapDistance = 40;
	unsigned WindowSnapDistance = 40;
	bool            bHooked = false;
	HHOOK            hhook = 0;
	HINSTANCE        hInst = 0;
	DWORD mod_count = 2;
	int modifiers[ MAX_NUM_MODIFIER_KEYS ] = { VK_MENU, VK_CONTROL };

#pragma data_seg ()

#pragma comment(linker, "/section:SHAREDDATA,rws")

RECT CalcWindowBorder( HWND hWnd );

BOOL APIENTRY DllMain( HINSTANCE hinstDLL,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            {
                // remember our instance handle
                hInst        = hinstDLL;
                ++iInstanceCount;

                break;
            }

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            --iInstanceCount;
            break;
    }

    return TRUE;
}

bool IgnoreWindow()
{
    // don't move or resize windows that are maximized
    WINDOWPLACEMENT wndpl;
    wndpl.length = sizeof(WINDOWPLACEMENT);
    ::GetWindowPlacement(hWnd, &wndpl);
    if(wndpl.showCmd == SW_SHOWMAXIMIZED)
    {
        return true;
    }

    // add other filters here...

    return false;
}

bool CheckModifierKeys()
{
    bool ret_val = true;

    unsigned i;
    /* Check each of the modifiers in turn */
    for( i = 0;
         i < mod_count;
         i++ )
    {
        ret_val = ret_val && ::GetAsyncKeyState( modifiers[ i ] );
    }

    return ret_val;
}

#include "wchar.h"

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

BOOL CALLBACK EnumWindowsProc( HWND p_hWnd, long lParam ) 
{
	/* Make sure it's a visible window (and not "Program Manager"), not the window we're moving and not minimised */
	if(( IsAltTabWindow( p_hWnd )) &&
		( p_hWnd != hWnd ) && 
		!IsIconic( p_hWnd ))
	{
		RECT winRect;
		GetWindowRect( p_hWnd, &winRect );
		RECT winBorder = CalcWindowBorder( p_hWnd );

		winRect.left += winBorder.left;
		winRect.right -= winBorder.right;
		winRect.top += winBorder.top;
		winRect.bottom -= winBorder.bottom;

		/* These points represent the corners of the visible window.  We use these to check
		   the (gross) visibility of the edges of the window so that we don't snap against
		   items which are not visible to the user.
		   
		   This does not catch the case where the corners are obscured, but part of the edge 
		   is still visible).
		*/
		const POINT topLeft = { winRect.left, winRect.top };
		const POINT bottomLeft = { winRect.left, winRect.bottom };
		const POINT topRight = { winRect.right, winRect.top };
		const POINT bottomRight = { winRect.right, winRect.bottom };

		if( ( x_snaps.next + 2 ) >= x_snaps.size )
		{
			/* Check that the top-most and bottom-most point of the left edge of the window are visible */
			if( ( WindowFromPoint( topLeft ) == p_hWnd ) || ( WindowFromPoint( bottomLeft ) == p_hWnd ) )
			{
				x_snaps.snap_list[ x_snaps.next ].snap = winRect.left;
				x_snaps.snap_list[ x_snaps.next ].rect.left = winRect.left - WindowSnapDistance;
				x_snaps.snap_list[ x_snaps.next ].rect.right = winRect.left + WindowSnapDistance;
				x_snaps.snap_list[ x_snaps.next ].rect.top = winRect.top;
				x_snaps.snap_list[ x_snaps.next ].rect.bottom = winRect.bottom;

				x_snaps.next++;
			}

			/* Check that the top-most and bottom-most point of the right edge of the window are visible */
			if( ( WindowFromPoint( topRight ) == p_hWnd ) || ( WindowFromPoint( bottomRight ) == p_hWnd ) )
			{
				x_snaps.snap_list[ x_snaps.next ].snap = winRect.right;
				x_snaps.snap_list[ x_snaps.next ].rect.left = winRect.right - WindowSnapDistance;
				x_snaps.snap_list[ x_snaps.next ].rect.right = winRect.right + WindowSnapDistance;
				x_snaps.snap_list[ x_snaps.next ].rect.top = winRect.top;
				x_snaps.snap_list[ x_snaps.next ].rect.bottom = winRect.bottom;
				x_snaps.next++;
			}
		}

		if( ( y_snaps.next + 2 ) >= y_snaps.size )
		{
			/* Check that the left-most and right-most point of the top edge of the window are visible */
			if( ( WindowFromPoint( topLeft ) == p_hWnd ) || ( WindowFromPoint( topRight ) == p_hWnd ) )
			{
				y_snaps.snap_list[ y_snaps.next ].snap = winRect.top;
				y_snaps.snap_list[ y_snaps.next ].rect.left = winRect.left;
				y_snaps.snap_list[ y_snaps.next ].rect.right = winRect.right;
				y_snaps.snap_list[ y_snaps.next ].rect.top = winRect.top - WindowSnapDistance;
				y_snaps.snap_list[ y_snaps.next ].rect.bottom = winRect.top + WindowSnapDistance;
				y_snaps.next++;
			}

			/* Check that the left-most and right-most point of the bottom edge of the window are visible */
			if( ( WindowFromPoint( bottomLeft ) == p_hWnd ) || ( WindowFromPoint( bottomRight ) == p_hWnd ) )
			{
				y_snaps.snap_list[ y_snaps.next ].snap = winRect.bottom;
				y_snaps.snap_list[ y_snaps.next ].rect.left = winRect.left;
				y_snaps.snap_list[ y_snaps.next ].rect.right = winRect.right;
				y_snaps.snap_list[ y_snaps.next ].rect.top = winRect.bottom - WindowSnapDistance;
				y_snaps.snap_list[ y_snaps.next ].rect.bottom = winRect.bottom + WindowSnapDistance;
				y_snaps.next++;
			}
		}
		

#if 0
		wchar_t local_str[ 2048 ] = L"";

		swprintf_s( local_str, _countof( local_str ), L"rect : %d.%d %d.%d  \n",

			winRect.left, winRect.top,
			winRect.right, winRect.bottom
		);
		RECT text_rect;
		text_rect.top = 100 + ( 20 * x_snaps.next );
		text_rect.left = 0;
		text_rect.right = 600;
		DrawText( GetDC( NULL ), local_str, -1, &text_rect, DT_SINGLELINE | DT_NOCLIP );
#endif


	}

	return TRUE;
}

BOOL CALLBACK EnumDispProc( HMONITOR hMon, HDC dcMon, RECT* pRcMon, LPARAM lParam )
{

#if 0
	/* This uses raw monitor dimensions */
	x_snaps.snap_list[ x_snaps.next ].snap = pRcMon->left;
	x_snaps.snap_list[ x_snaps.next ].min = pRcMon->left - MonitorSnapDistance;
	x_snaps.snap_list[ x_snaps.next ].max = pRcMon->left + MonitorSnapDistance;
	x_snaps.snap_list[ x_snaps.next ].val_start = pRcMon->top;
	x_snaps.snap_list[ x_snaps.next ].val_end = pRcMon->bottom;
	x_snaps.next++;

	x_snaps.snap_list[ x_snaps.next ].snap = pRcMon->right;
	x_snaps.snap_list[ x_snaps.next ].min = pRcMon->right - MonitorSnapDistance;
	x_snaps.snap_list[ x_snaps.next ].max = pRcMon->right + MonitorSnapDistance;
	x_snaps.snap_list[ x_snaps.next ].val_start = pRcMon->top;
	x_snaps.snap_list[ x_snaps.next ].val_end = pRcMon->bottom;
	x_snaps.next++;

	y_snaps.snap_list[ y_snaps.next ].snap = pRcMon->top;
	y_snaps.snap_list[ y_snaps.next ].min = pRcMon->top - MonitorSnapDistance;
	y_snaps.snap_list[ y_snaps.next ].max = pRcMon->top + MonitorSnapDistance;
	y_snaps.snap_list[ y_snaps.next ].val_start = pRcMon->left;
	y_snaps.snap_list[ y_snaps.next ].val_end = pRcMon->right;
	y_snaps.next++;

	y_snaps.snap_list[ y_snaps.next ].snap = pRcMon->bottom;
	y_snaps.snap_list[ y_snaps.next ].min = pRcMon->bottom - MonitorSnapDistance;
	y_snaps.snap_list[ y_snaps.next ].max = pRcMon->bottom + MonitorSnapDistance;
	y_snaps.snap_list[ y_snaps.next ].val_start = pRcMon->left;
	y_snaps.snap_list[ y_snaps.next ].val_end = pRcMon->right;
	y_snaps.next++;
#else
	MONITORINFO monitorInfo;
	  
	/* We want to snap to the working area of the monitor, not the raw monitor */
	monitorInfo.cbSize = sizeof( MONITORINFO );
	GetMonitorInfo( hMon, &monitorInfo );

	if( ( x_snaps.next + 2 ) >= x_snaps.size )
	{

		x_snaps.snap_list[ x_snaps.next ].snap = monitorInfo.rcWork.left;
		x_snaps.snap_list[ x_snaps.next ].rect.left = monitorInfo.rcWork.left - MonitorSnapDistance;
		x_snaps.snap_list[ x_snaps.next ].rect.right = monitorInfo.rcWork.left + MonitorSnapDistance;
		x_snaps.snap_list[ x_snaps.next ].rect.top = monitorInfo.rcWork.top;
		x_snaps.snap_list[ x_snaps.next ].rect.bottom = monitorInfo.rcWork.bottom;
		x_snaps.next++;

		x_snaps.snap_list[ x_snaps.next ].snap = monitorInfo.rcWork.right;
		x_snaps.snap_list[ x_snaps.next ].rect.left = monitorInfo.rcWork.right - MonitorSnapDistance;
		x_snaps.snap_list[ x_snaps.next ].rect.right = monitorInfo.rcWork.right + MonitorSnapDistance;
		x_snaps.snap_list[ x_snaps.next ].rect.top = monitorInfo.rcWork.top;
		x_snaps.snap_list[ x_snaps.next ].rect.bottom = monitorInfo.rcWork.bottom;
		x_snaps.next++;
	}

	if( ( y_snaps.next + 2 ) >= y_snaps.size )
	{
		y_snaps.snap_list[ y_snaps.next ].snap = monitorInfo.rcWork.top;
		y_snaps.snap_list[ y_snaps.next ].rect.left = monitorInfo.rcWork.left;
		y_snaps.snap_list[ y_snaps.next ].rect.right = monitorInfo.rcWork.right;
		y_snaps.snap_list[ y_snaps.next ].rect.top = monitorInfo.rcWork.top - MonitorSnapDistance;
		y_snaps.snap_list[ y_snaps.next ].rect.bottom = monitorInfo.rcWork.top + MonitorSnapDistance;
		y_snaps.next++;

		y_snaps.snap_list[ y_snaps.next ].snap = monitorInfo.rcWork.bottom;
		y_snaps.snap_list[ y_snaps.next ].rect.left = monitorInfo.rcWork.left;
		y_snaps.snap_list[ y_snaps.next ].rect.right = monitorInfo.rcWork.right;
		y_snaps.snap_list[ y_snaps.next ].rect.top = monitorInfo.rcWork.bottom - MonitorSnapDistance;
		y_snaps.snap_list[ y_snaps.next ].rect.bottom = monitorInfo.rcWork.bottom + MonitorSnapDistance;
		y_snaps.next++;
	}
#endif

	return TRUE;
}

void UpdateSnaps()
{
	x_snaps.next = 0;
	y_snaps.next = 0;
	if( MonitorSnapDistance > 0 )
	{
		EnumDisplayMonitors( 0, 0, EnumDispProc, NULL );
	}
	if( WindowSnapDistance > 0 )
	{
		EnumWindows( EnumWindowsProc, NULL );
	}
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

LONG YPosIncSnap( POINT ptRel, LONG width, LONG height, bool top, bool bottom )
{
	LONG retVal = ptRel.y;

	RECT topFace;
	topFace.left = ptRel.x;
	topFace.right = ptRel.x + width;
	topFace.top = ptRel.y;
	topFace.bottom = ptRel.y + 1;

	RECT bottomFace;
	bottomFace.left = ptRel.x;
	bottomFace.right = ptRel.x + width;
	bottomFace.top = ptRel.y + height;
	bottomFace.bottom = ptRel.y + height + 1;

	for( unsigned i = 0; i < y_snaps.next; i++ )
	{
		RECT out;
		if( top )
		{
			if( IntersectRect( &out, &( y_snaps.snap_list[ i ].rect ), &topFace ) )
			{
				retVal = y_snaps.snap_list[ i ].snap - border.top;
				break;
			}
		}

		if( bottom )
		{
			if( IntersectRect( &out, &( y_snaps.snap_list[ i ].rect ), &bottomFace ) )
			{
				retVal = y_snaps.snap_list[ i ].snap + border.bottom - height;
				break;
			}
		}
	}

	return retVal;
}

LONG XPosIncSnap( POINT ptRel, LONG width, LONG height, bool left, bool right )
{
	LONG retVal = ptRel.x;

	RECT leftFace;
	leftFace.left = ptRel.x;
	leftFace.right = ptRel.x+1;
	leftFace.top = ptRel.y;
	leftFace.bottom = ptRel.y + height;

	RECT rightFace;
	rightFace.left = ptRel.x + width;
	rightFace.right = ptRel.x + width + 1;
	rightFace.top = ptRel.y;
	rightFace.bottom = ptRel.y + height;

	for( unsigned i = 0; i < x_snaps.next; i++ )
	{
		RECT out;

		if( left )
		{
			if( IntersectRect( &out, &( x_snaps.snap_list[ i ].rect ), &leftFace ) )
			{
				retVal = x_snaps.snap_list[ i ].snap - border.left;
				break;
			}
		}

		if( right )
		{
			if( IntersectRect( &out, &( x_snaps.snap_list[ i ].rect ), &rightFace ) )
			{
				retVal = x_snaps.snap_list[ i ].snap + border.right - width;
				break;
			}
		}
	}
	return retVal;
}

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam )
{
    if (nCode != HC_ACTION)
    {
        return CallNextHookEx(hhook, nCode, wParam, lParam);
    }

    MOUSEHOOKSTRUCT &mhs    = *(MOUSEHOOKSTRUCT*)wParam;

    switch(wParam)
    {
        case WM_RBUTTONDOWN :
        case WM_LBUTTONDOWN :
            {
                bool modifierState = CheckModifierKeys();
                if (! modifierState)
                {
                    return CallNextHookEx(hhook, nCode, wParam, lParam);
                }

                // remember where the user started dragging the mouse
                ::GetCursorPos(&ptMouseInit);
                hWnd = ::WindowFromPoint(ptMouseInit);

                // walk up the window hierarchie to find the corresponding
                // top level window. commment this block and enjoy resizing
                // all sorts of widgets :)
                while (hWnd != 0)
                {
                    style       = ::GetWindowLong(hWnd,GWL_STYLE);
                    const int x = style & (WS_POPUP|WS_CHILD);

                    if ( (x == WS_OVERLAPPED) ||
                         (x == WS_POPUP)
                        ) break;

                    // we also want to manipulate mdi childs that
                    // aren't maximized
                    if (
                            (! (style & WS_MAXIMIZE)) &&
                            (::GetWindowLong(hWnd,GWL_EXSTYLE) & WS_EX_MDICHILD)
                        ) break;

                    hWnd = ::GetParent(hWnd);
                }

                if (IgnoreWindow())
                {
                    opMode = O_NONE;
                    return CallNextHookEx(hhook, nCode, wParam, lParam);
                }

                // check if the modifier key is pressed while a mouse button is pressed
                // and switch to the appropriate mode
                switch (wParam)
                {
                    case WM_RBUTTONDOWN :
                        opMode = O_SIZE;
                        break;
                    case WM_LBUTTONDOWN :
                        opMode = O_MOVE;
                        break;
                    default :
                        opMode = O_NONE;
                        return CallNextHookEx(hhook, nCode, wParam, lParam);
                }

                if (hWnd == 0)
                {
                    opMode = O_NONE;
                    return CallNextHookEx(hhook, nCode, wParam, lParam);
                }

                HWND parent = ::GetParent(hWnd);

                // remember the window size and position
                ::GetWindowRect(hWnd,&rWndInit);
                if ( (parent) &&
                     ( (style & WS_POPUP) == 0) )
                {
                    ::ScreenToClient(parent,(POINT*) &rWndInit.left);
                    ::ScreenToClient(parent,(POINT*) &rWndInit.right);
                }

				UpdateSnaps();
				border = CalcWindowBorder( hWnd );

                // we're getting serious - capture the mouse
                SetCapture(hWnd);

                return 1;
            }

        case WM_MOUSEMOVE :
            {
                if (opMode == O_NONE)
                {
                    return CallNextHookEx(hhook, nCode, wParam, lParam);
                } else
                {
                    bool modifierState            = CheckModifierKeys();

                    // check if modifier key is released
                    // while dragging or sizing a window
                    if (! modifierState)
                    {
                        ReleaseCapture();
                        return CallNextHookEx(hhook, nCode, wParam, lParam);
                    }
                }

                switch (opMode)
                {
                    case O_MOVE :
                        {
                            // we're dragging a window - calc the relative movement and
                            // reposition the window
                            POINT ptRel;
                            ::GetCursorPos(&ptRel);

                            ptRel.x    -= ptMouseInit.x;
                            ptRel.y    -= ptMouseInit.y;

                            ptRel.x += rWndInit.left;
                            ptRel.y += rWndInit.top;

#if 0
							wchar_t local_str[ 1024 ] = L"";
							swprintf_s( local_str, _countof( local_str ), L"ptRel : %d.%d   \n",
								ptRel.x, ptRel.y );
							RECT text_rect;
							text_rect.top = 50;
							text_rect.left = 0;
							DrawText( GetDC( NULL ), local_str, -1, &text_rect, DT_SINGLELINE | DT_NOCLIP );
#endif
							const LONG width = rWndInit.right - rWndInit.left;
							const LONG height = rWndInit.bottom - rWndInit.top;

							ptRel.x = XPosIncSnap( ptRel, width, height, true, true );
							ptRel.y = YPosIncSnap( ptRel, width, height, true, true );

#if 0
							swprintf_s( local_str, _countof( local_str ), L"ptRel : %d.%d   \n",
								ptRel.x, ptRel.y );
							text_rect.top = 70;
							text_rect.left = 0;
							DrawText( GetDC( NULL ), local_str, -1, &text_rect, DT_SINGLELINE | DT_NOCLIP );
#endif

                            ::SetWindowPos(hWnd,0,ptRel.x,ptRel.y,0,0,
                                SWP_ASYNCWINDOWPOS|SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOSIZE|SWP_NOZORDER);

                            return 1;
                        }
                    case O_SIZE :
                        {
                            // we're resizing a window - calc the relative movement
                            // and resize the window if it has a sizebox. disable
                            // this check and resize all windows :)
                            if (style & WS_SIZEBOX)
                            {
                                const LONG width    = (rWndInit.right - rWndInit.left);
                                const LONG height    = (rWndInit.bottom - rWndInit.top);

                                POINT mouse;
                                POINT pos;
                                POINT size;
                                ::GetCursorPos(&mouse);

                                const LONG dmousex    = mouse.x - ptMouseInit.x;
                                const LONG dmousey    = mouse.y - ptMouseInit.y;

                                if (ptMouseInit.x > rWndInit.left + (width/2))
                                {
                                    pos.x    = rWndInit.left;
                                    size.x    = dmousex + width;

									POINT ptRel;
									ptRel.x = rWndInit.left + size.x;
									ptRel.y = rWndInit.top;

									size.x = XPosIncSnap( ptRel, 0, dmousey + height, false, true ) - pos.x;
                                } else 
                                {
                                    pos.x    = rWndInit.left + dmousex;
                                    size.x    = width - dmousex;

									POINT ptRel;
									ptRel.x = pos.x;
									ptRel.y = rWndInit.top;

									pos.x = XPosIncSnap( ptRel, 0, dmousey + height, true, false );

									/* Compensate the width to deal with the position snap */
									size.x += ptRel.x - pos.x;
								}

                                if (ptMouseInit.y > rWndInit.top + (height/2))
                                {
                                    pos.y    = rWndInit.top;
                                    size.y    = dmousey + height;

									POINT ptRel;
									ptRel.x = rWndInit.left;
									ptRel.y = rWndInit.top + size.y;

								 	size.y = YPosIncSnap( ptRel, dmousex + width, 0, false, true ) - pos.y;

                                } else 
                                {
                                    pos.y    = rWndInit.top + dmousey;
                                    size.y    = height - dmousey;

									POINT ptRel;
									ptRel.x = pos.x;
									ptRel.y = pos.y;

									pos.y = YPosIncSnap( ptRel, dmousex + width, 0, true, false );
									/* Compensate the height to deal with the position snap */
									size.y += ptRel.y - pos.y;

                                }

                                ::SetWindowPos(hWnd,0,pos.x,pos.y,size.x,size.y,
                                    SWP_ASYNCWINDOWPOS|SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER);
                            }

                            return 1;
                        }
                }
            }

            default :
            {
                if (opMode != O_NONE)
                {
                    // return to nop mode in all other cases
                    ReleaseCapture();
                    opMode = O_NONE;
                    return 1;
                }
                else
                {
                    return CallNextHookEx( hhook, nCode, wParam, lParam );
                }
            }
    }
}

DLL_EXPORT void SetMouseHook(void)
{
    if (!bHooked)
    {
        hhook        = SetWindowsHookEx(WH_MOUSE, (HOOKPROC)MouseProc, hInst, (DWORD)NULL);
        bHooked        = true;
    }
}

DLL_EXPORT    void RemoveMouseHook(void)
{
    if(bHooked)
    {
        UnhookWindowsHookEx(hhook);
        bHooked = false;
    }
}

DLL_EXPORT int GetInstanceCount()
{
    return iInstanceCount;
}

DLL_EXPORT void SetScreenSnap(int val)
{
	MonitorSnapDistance = val;
}

DLL_EXPORT void SetWindowSnap( int val )
{
	WindowSnapDistance = val;
}

DLL_EXPORT void SetModifiers( int count, int mods[] )
{
    
    if( ( count > 1 ) && ( count < MAX_NUM_MODIFIER_KEYS ) )
    {
        int i;
        for( i = 0; i < count; i++ )
        {
            modifiers[ i ] = mods[ i ];
        }
        mod_count = count;
    }
}

