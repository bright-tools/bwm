#include "wmmousedll.h"
#include <windows.h>

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

    // the current mode of operation
    OpMode            opMode;
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

                            ::SetWindowPos(hWnd,0,ptRel.x,ptRel.y,0,0,
                                SWP_ASYNCWINDOWPOS|SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOSIZE|SWP_NOZORDER);

                            return 1;
                        }
                    case O_SIZE :
                        {
                            // we're resizing a window - calc the relatvie movement
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
                                } else 
                                {
                                    pos.x    = rWndInit.left + dmousex;
                                    size.x    = width - dmousex;
                                }

                                if (ptMouseInit.y > rWndInit.top + (height/2))
                                {
                                    pos.y    = rWndInit.top;
                                    size.y    = dmousey + height;
                                } else 
                                {
                                    pos.y    = rWndInit.top + dmousey;
                                    size.y    = height - dmousey;
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

