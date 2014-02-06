#include <windows.h>
#include "resource.h"
#include "wmmousedll/wmmouse.h"

//
// some global data
//
HWND				ghWnd;
NOTIFYICONDATA		nid;
HINSTANCE			hAppInstance;

static const UINT WM_TRAY_NOTIFY	= (WM_APP + 1000);
static WCHAR szAppName[]		= L"Window Manager";

//
// spawn a message box
//
void Message(const char* message)
{
	MessageBoxA(GetDesktopWindow(),message,"Window Manager", MB_OK);
}

//
// manage the tray icon
//
bool InitTrayIcon()
{
	nid.cbSize				= sizeof(NOTIFYICONDATA) ;
	nid.hWnd				= ghWnd;
	nid.uID					= 0 ;
	nid.uFlags				= NIF_MESSAGE | NIF_ICON | NIF_TIP ;	
	nid.uCallbackMessage	= WM_TRAY_NOTIFY;  
	wcscpy(nid.szTip,szAppName);
	nid.hIcon				= ::LoadIcon(hAppInstance,MAKEINTRESOURCE(IDI_TRAY)); 
	
	if (Shell_NotifyIcon (NIM_ADD,&nid) != TRUE)
	{
		Message("Unable to create tray icon.");
		return false;
	}

	return true;
}

bool RemoveTrayIcon()
{
	if (Shell_NotifyIcon (NIM_DELETE,&nid) != TRUE)
	{
		Message("Unable to remove tray icon.");
		return false;
	}

	return true;

}

//
// manage the about dialog box
//
BOOL CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if (uMsg == WM_COMMAND)
	{
		WORD wID		 = LOWORD(wParam);
		if (wID == IDOK) DestroyWindow(hwndDlg);
	}

	return 0;
}

void AboutDlg()
{
	DialogBox(hAppInstance,MAKEINTRESOURCE(IDD_ABOUT),NULL,DialogProc);
}

//
// manage the context menu
//
void DoContextMenu()
{
	HMENU hMenu =LoadMenu(hAppInstance,MAKEINTRESOURCE(IDR_TRAY));
	if (hMenu == NULL)
	{
		Message("Unable to load menu ressource.");
		return;
	}

	HMENU hSubMenu = GetSubMenu(hMenu,0);
	if (hSubMenu == NULL)
	{
		Message("Unable to find popup mennu.");
		return;
	}

	// get the cursor position
	POINT pt;
	GetCursorPos (&pt) ;

	SetForegroundWindow(ghWnd);
	int cmd = TrackPopupMenu(hSubMenu,TPM_RETURNCMD|TPM_LEFTALIGN|TPM_RIGHTBUTTON,
							pt.x,pt.y,0,ghWnd,NULL);				
	DeleteObject(hMenu);

	switch (cmd)
	{
		case ID_WMEXIT :
		{
			PostQuitMessage(0);
			break;						
		}
		case ID_WMABOUT :
		{
			AboutDlg();
			break;
		}
	}
}

//
// manage the main window
//
LONG WINAPI MainWndProc ( HWND    hWnd, UINT    uMsg, WPARAM  wParam, LPARAM  lParam) 
{ 
	switch (uMsg)
	{
		case WM_DESTROY :
		{
			PostQuitMessage (0);
			return 0;
		}
		case WM_TRAY_NOTIFY :
		{
			if (lParam == WM_RBUTTONDOWN) 
				DoContextMenu();
			return 0;
		}
	}
	
	return DefWindowProc (hWnd, uMsg, wParam, lParam); 
} 

bool InitWindow()
{
	// register the frame class
	WNDCLASS wndclass;
    wndclass.style         = 0; 
    wndclass.lpfnWndProc   = (WNDPROC)MainWndProc; 
    wndclass.cbClsExtra    = 0; 
    wndclass.cbWndExtra    = 0; 
    wndclass.hInstance     = hAppInstance; 
    wndclass.hIcon         = 0; 
    wndclass.hCursor       = LoadCursor (NULL,IDC_ARROW); 
    wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1); 
    wndclass.lpszMenuName  = NULL; 
    wndclass.lpszClassName = szAppName;
 
    if (!RegisterClass (&wndclass))
	{
		Message("Unable to register the window class.");
		return false;
	}
 
    // create the frame 
    ghWnd = CreateWindow (szAppName, szAppName, 
         WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 
             CW_USEDEFAULT, 
             CW_USEDEFAULT, 
             640, 
             480, 
             NULL, 
             NULL, 
             hAppInstance, 
             NULL); 
 
    // make sure window was created
    if (!ghWnd) 
	{
		Message("Unable to create the window.");
		return false; 
	}

	return true;
}

//
// init
//
bool Init(LPSTR lpCmdLine)
{
	// try to load wmmouse.dll
	if (!WMMouse::Init())
	{
		Message("Unable to init wmmouse.dll.");
		return false;
	}

	// check number of instances
	if (WMMouse::GetInstanceCount() == 1)
	{
		// hook the mouse
		WMMouse::SetMouseHook();
		return true;
	} else 
	{
		// mouse was already hooked
		return false;
	}
}

// 
// our main loop
//
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
{
	MSG msg;
	hAppInstance = hInstance;
	if (!Init(lpCmdLine)) 
	{
		return 0;
	}

	// create a dummy window to receive WM_QUIT message
	InitWindow();

	// create the tray icon
	if( InitTrayIcon() )
	{

		// just get and dispatch messages until we're killed
		while (GetMessage(&msg,0,0,0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		};

	}

	// remove our mouse hook before saying goodbye
	WMMouse::RemoveMouseHook();
	WMMouse::Done();

	// remove our tray icon
	RemoveTrayIcon();

    return msg.wParam;
}
 
