#include "wmmousedll.h"
#include <Dwmapi.h>
#include <windows.h>

#define MAX_NUM_MODIFIER_KEYS 10U

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
	snap_t* snap_list;
	unsigned size;
	unsigned next;
} snap_info_t;

#define DEFAULT_SNAP_ALLOC 20
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

	snap_info_t x_snaps = { NULL, 0, 0 };
	snap_info_t y_snaps = { NULL, 0, 0 };

	unsigned MonitorSnapDistance = 40;
#pragma data_seg ()

#pragma comment(linker, "/section:SHAREDDATA,rws")

bool            bHooked = false;
HHOOK            hhook    = 0;
HINSTANCE        hInst    = 0;
DWORD mod_count = 2;
int modifiers[ MAX_NUM_MODIFIER_KEYS ] = { VK_MENU, VK_CONTROL };

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

void ReAllocSnap( snap_info_t* snap_info )
{
    if( snap_info->size == 0 )
    {
        snap_info->snap_list = (snap_t*)malloc( sizeof( snap_info_t ) * DEFAULT_SNAP_ALLOC );
        snap_info->size = DEFAULT_SNAP_ALLOC;
    }
    else
    {
        snap_info->snap_list = (snap_t*)realloc( snap_info->snap_list, snap_info->size + ( sizeof( snap_info_t ) * SNAP_ALLOC_GROW ));
        snap_info->size += SNAP_ALLOC_GROW;
    }
}

BOOL CALLBACK EnumDispProc( HMONITOR hMon, HDC dcMon, RECT* pRcMon, LPARAM lParam )
{

    if( x_snaps.next+2 >= x_snaps.size )
    {
        ReAllocSnap( &x_snaps );
    }
    if( y_snaps.next+2 >= y_snaps.size )
	{
	ReAllocSnap( &y_snaps );
	}

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

#endif

	return TRUE;
}

void UpdateSnaps()
{
	x_snaps.next = 0;
	y_snaps.next = 0;
	EnumDisplayMonitors( 0, 0, EnumDispProc, NULL );
}

void CalcWindowBorder()
{
	RECT rect, frame;
	GetWindowRect( hWnd, &rect );
	DwmGetWindowAttribute( hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &frame, sizeof( RECT ) );

	border.left = frame.left - rect.left;
	border.top = frame.top - rect.top;
	border.right = rect.right - frame.right;
	border.bottom = rect.bottom - frame.bottom;
}

LONG YPosIncSnap( POINT ptRel )
{
	LONG retVal = ptRel.y;
	LONG height = rWndInit.bottom - rWndInit.top;
	POINT ptOther = ptRel;
	ptOther.y += height;

	for( unsigned i = 0; i < y_snaps.next; i++ )
	{
		if( PtInRect( &(y_snaps.snap_list[i].rect), ptRel ))
		{
			retVal = y_snaps.snap_list[ i ].snap - border.top;
			break;
		}

		if( PtInRect( &( y_snaps.snap_list[ i ].rect ), ptOther ) )
		{
			retVal = y_snaps.snap_list[ i ].snap + border.bottom - height;
			break;
		}
	}

	return retVal;
}

LONG XWidthIncSnap( POINT ptRel, bool left )
{
	LONG retVal = ptRel.x;

	for( unsigned i = 0; i < x_snaps.next; i++ )
	{
		if( PtInRect( &( x_snaps.snap_list[ i ].rect ), ptRel ) )
		{
			retVal = x_snaps.snap_list[ i ].snap;
			if( left )
			{
				retVal -= border.left;
			}
			else
			{
				retVal += border.right;
			}
			break;
		}
	}
	return retVal;
}

LONG YHeightIncSnap( POINT ptRel, bool top )
{
	LONG retVal = ptRel.y;

	for( unsigned i = 0; i < y_snaps.next; i++ )
	{
		if( PtInRect( &( y_snaps.snap_list[ i ].rect ), ptRel ) )
		{
			retVal = y_snaps.snap_list[ i ].snap;
			if( top )
			{
				retVal -= border.top;
			}
			else
			{
				retVal += border.bottom;
			}
			break;
		}
	}
	return retVal;
}

LONG XPosIncSnap( POINT ptRel )
{
	LONG retVal = ptRel.x;
	const LONG width = rWndInit.right - rWndInit.left;
	POINT ptOther = ptRel;
	ptOther.x += width;

	for( unsigned i = 0; i < x_snaps.next; i++ )
	{
		if( PtInRect( &( x_snaps.snap_list[ i ].rect ), ptRel ) )
		{
			retVal = x_snaps.snap_list[ i ].snap - border.left;
			break;
		}
		if( PtInRect( &( x_snaps.snap_list[ i ].rect ), ptOther ) )
		{
			retVal = x_snaps.snap_list[ i ].snap + border.right - width;
			break;
		}
	}
	return retVal;
}

#include "wchar.h"

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
				CalcWindowBorder();

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
                            POINT ptRel, ptRelOrig;
                            ::GetCursorPos(&ptRel);

                            ptRel.x    -= ptMouseInit.x;
                            ptRel.y    -= ptMouseInit.y;

                            ptRel.x += rWndInit.left;
                            ptRel.y += rWndInit.top;

							ptRelOrig = ptRel;
							ptRel.x = XPosIncSnap( ptRel );
							ptRel.y = YPosIncSnap( ptRelOrig );

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
									ptRel.x = pos.x + size.x;
									ptRel.y = mouse.y;

									size.x = XWidthIncSnap( ptRel, false ) - pos.x;
                                } else 
                                {
                                    pos.x    = rWndInit.left + dmousex;
                                    size.x    = width - dmousex;

									POINT ptRel;
									ptRel.x = pos.x;
									ptRel.y = mouse.y;
									pos.x = XWidthIncSnap( ptRel, true );

									/* Compensate the width to deal with the position snap */
									size.x += ptRel.x - pos.x;
								}

                                if (ptMouseInit.y > rWndInit.top + (height/2))
                                {
                                    pos.y    = rWndInit.top;
                                    size.y    = dmousey + height;

									POINT ptRel;
									ptRel.x = mouse.x;
									ptRel.y = pos.y + size.y;

									size.y = YHeightIncSnap( ptRel, false ) - pos.y;

                                } else 
                                {
                                    pos.y    = rWndInit.top + dmousey;
                                    size.y    = height - dmousey;

									POINT ptRel;
									ptRel.x = mouse.x;
									ptRel.y = pos.y;

									pos.y = YHeightIncSnap( ptRel, true );
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

