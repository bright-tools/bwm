#if !defined WINUTILS_HPP
#define      WINUTILS_HPP

#include <Windows.h>

bool IgnoreWindow();
BOOL IsAltTabWindow( HWND hwnd );
RECT CalcWindowBorder( HWND p_hWnd );
HWND GetTopLevelWindow( HWND p_hWnd );

#endif