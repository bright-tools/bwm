/*
File: wmmousedll.cpp
Project: BWM - https://github.com/bright-tools/bwm

Copyright 2016 Bright Silence Limited
Copyright 2003-2005 Markus Rollman

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

See the License for the specific language governing permissions and
limitations under the License.
*/

#include "wmmousedll.h"
#include "MouseHistory.hpp"
#include "WinUtils.hpp"
#include "Snap.hpp"
#include <windows.h>

#include "wchar.h"

#define MAX_NUM_MODIFIER_KEYS 10U

// enum possible states the window manager is in
enum OpMode
{
    O_NONE,    // we're currently doing nothing
    O_MOVE,    // we're dragging a window
    O_SIZE    // we're sizing a window
};

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

	MouseHistory mouseHistory;

	enum MouseButton MButtonMove = M_LEFT;
	enum MouseButton MButtonResize = M_RIGHT;
	enum MouseButton MButtonMinimise = M_LEFT;
	enum MouseButton MButtonMaximise = M_RIGHT;

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

bool HandleDoubleClick( const enum MouseButton thisButton )
{
	bool handled = false;
	WINDOWPLACEMENT wndpl;
	wndpl.length = sizeof( WINDOWPLACEMENT );
	::GetWindowPlacement( hWnd, &wndpl );
	if( thisButton == MButtonMaximise )
	{
		if( wndpl.showCmd == SW_SHOWMAXIMIZED )
		{
			wndpl.showCmd = SW_SHOWNORMAL;
		}
		else
		{
			wndpl.showCmd = SW_SHOWMAXIMIZED;
		}
		handled = true;
	}
	else if( thisButton == MButtonMinimise )
	{
		wndpl.showCmd = SW_MINIMIZE;
		handled = true;
	}

	if( handled )
	{
		::SetWindowPlacement( hWnd, &wndpl );
	}

	return handled;
}

enum MouseButton MouseButtonFromWPARAM( WPARAM wParam )
{
	enum MouseButton thisButton;
	switch( wParam )
	{
		case WM_RBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
			thisButton = M_RIGHT;
			break;
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONDOWN:
			thisButton = M_LEFT;
			break;
		default:
			thisButton = M_NONE;
			break;
	}
	return thisButton;
}

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam )
{
    if (nCode != HC_ACTION)
    {
        return CallNextHookEx(hhook, nCode, wParam, lParam);
    }

	MOUSEHOOKSTRUCT &mhs = *(MOUSEHOOKSTRUCT*)lParam;

    switch(wParam)
    {
		case WM_RBUTTONDBLCLK:
		case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN :
        case WM_LBUTTONDOWN :
			{
				bool modifierState = CheckModifierKeys();
				if( !modifierState )
				{
					return CallNextHookEx( hhook, nCode, wParam, lParam );
				}

				enum MouseButton thisButton = MouseButtonFromWPARAM( wParam );
				if( thisButton == M_NONE )
				{
					opMode = O_NONE;
					return CallNextHookEx( hhook, nCode, wParam, lParam );
				}

				// remember where the user started dragging the mouse
				::GetCursorPos( &ptMouseInit );
				hWnd = ::WindowFromPoint( ptMouseInit );
				DWORD winStyle = ::GetClassLong( hWnd, GCL_STYLE );

				hWnd = GetTopLevelWindow( hWnd );

				/* If the window style has CS_DBLCLKS then we'll receive WM_LBUTTONDBLCLK and WM_RBUTTONDBLCLK messages
				   If not, we have to monitor for double clicks manually */
				if( (( winStyle & CS_DBLCLKS ) && (( wParam == WM_LBUTTONDBLCLK ) || (wParam == WM_RBUTTONDBLCLK ))) || 
					mouseHistory.IsDoubleClick( thisButton, mhs ) )
				{
					if( HandleDoubleClick( thisButton ) )
					{
						return 1;
					}
					else
					{
						return CallNextHookEx( hhook, nCode, wParam, lParam );
					}
				}

				if( IgnoreWindow() )
				{
					opMode = O_NONE;
					return CallNextHookEx( hhook, nCode, wParam, lParam );
				}

				// check if the modifier key is pressed while a mouse button is pressed
				// and switch to the appropriate mode
				if( MButtonMove == thisButton )
				{
					opMode = O_MOVE;
				}
				else if( MButtonResize == thisButton )
				{
					opMode = O_SIZE;
				}
				else
				{
					hWnd = 0;
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
#if 0
				DrawSnaps();
#endif
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
						opMode = O_NONE;
                        ReleaseCapture();
						return CallNextHookEx( hhook, nCode, wParam, lParam );
                    }
                }
				SetCursor( LoadCursor( NULL, IDC_SIZEALL ) );

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
                                const LONG initialWidth    = (rWndInit.right - rWndInit.left);
                                const LONG initialHeight    = (rWndInit.bottom - rWndInit.top);
								RECT currentPos;
								::GetWindowRect( hWnd, &currentPos );
								const LONG currentHeight = ( currentPos.bottom - currentPos.top );
								const LONG currentWidth = ( currentPos.right - currentPos.left );

                                POINT mouse;
                                POINT pos;
                                POINT size;
                                ::GetCursorPos(&mouse);

                                const LONG dmousex    = mouse.x - ptMouseInit.x;
                                const LONG dmousey    = mouse.y - ptMouseInit.y;

								/* Are we resizing in the right-half of the window? */
                                if (ptMouseInit.x > rWndInit.left + (initialWidth/2))
                                {
									/* Left-hand edge of window is fixed */
                                    pos.x    = rWndInit.left;
									/* Width varies based on mouse delta */
                                    size.x    = dmousex + initialWidth;

									POINT ptRel;
									ptRel.x = rWndInit.left;
									ptRel.y = rWndInit.top;

									size.x = XPosIncSnap( ptRel, size.x, currentHeight, false, true ) - pos.x;
                                } else 
                                {
									/* Left hand edge of window moves with mouse */
                                    pos.x    =  rWndInit.left + dmousex;
									/* Size adjusts to keep right-hand edge in place */
                                    size.x    = initialWidth - dmousex;

									POINT ptRel;
									ptRel.x = pos.x;
									ptRel.y = rWndInit.top;

									pos.x = XPosIncSnap( ptRel, 0, currentHeight, true, false );

									/* Compensate the width to deal with the fact that the left-hand edge
									   is moving (but not the right-hand edge) */
									size.x += ptRel.x - pos.x;
								}

								/* Are we resizing in the bottom-half of the window? */
                                if (ptMouseInit.y > rWndInit.top + (initialHeight/2))
                                {
									/* Top edge of window is fixed */
                                    pos.y    = rWndInit.top;
									/* Height varies based on mouse delta */
                                    size.y    = dmousey + initialHeight;

									POINT ptRel;
									ptRel.x = pos.x;
									ptRel.y = rWndInit.top;

								 	size.y = YPosIncSnap( ptRel, currentWidth, size.y, false, true ) - pos.y;

                                } else 
                                {
									/* Top edge of window moves with mouse */
                                    pos.y    = rWndInit.top + dmousey;
									/* Height of window adjusts to keep right-hand edge in place */
                                    size.y    = initialHeight - dmousey;

									POINT ptRel;
									ptRel.x = pos.x;
									ptRel.y = pos.y;

									pos.y = YPosIncSnap( ptRel, currentWidth, 0, true, false );

									/* Compensate the width to deal with the fact that the left-hand edge
									is moving (but not the right-hand edge) */
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
		ChangeWindowMessageFilter( WH_MOUSE, MSGFLT_ADD );
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

DLL_EXPORT void SetMoveMButton( int val )
{
	MButtonMove = (enum MouseButton)val;
}

DLL_EXPORT void SetResizeMButton( int val )
{
	MButtonResize = ( enum MouseButton )val;
}

DLL_EXPORT void SetMinimiseMButton( int val )
{
	MButtonMinimise = ( enum MouseButton )val;
}

DLL_EXPORT void SetMaximiseMButton( int val )
{
	MButtonMaximise = ( enum MouseButton )val;
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

