/*
File: Snap.cpp
Project: BWM - https://github.com/bright-tools/bwm

Copyright 2016 Bright Silence Limited

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

#include "Snap.hpp"
#include "wmmousedll.h"
#include "WinUtils.hpp"

BOOL CALLBACK EnumWindowsProc( HWND p_hWnd, long lParam )
{
	/* Make sure it's a visible window (and not "Program Manager"), not the window we're moving and not minimised */
	if( ( IsAltTabWindow( p_hWnd ) ) &&
		( p_hWnd != hWnd ) &&
		!IsIconic( p_hWnd ) )
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

	/* Monitor edge snapping is disabled when the distance is set to zero */
	if( MonitorSnapDistance > 0 )
	{
		EnumDisplayMonitors( 0, 0, EnumDispProc, NULL );
	}

	/* Window edge snapping is disabled when the distance is set to zero */
	if( WindowSnapDistance > 0 )
	{
		EnumWindows( EnumWindowsProc, NULL );
	}
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
	leftFace.right = ptRel.x + 1;
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