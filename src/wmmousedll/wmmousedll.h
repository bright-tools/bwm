#ifndef __WMMOUSEDLL_H__
#define __WMMOUSEDLL_H__

#include <Windows.h>

#pragma once

#define MAX_SNAPS 200U

typedef void (*SETKBHOOK) ();
typedef void (*REMOVEKBHOOK) ();
typedef int  (*GETINSTANCECOUNT) ();
typedef void (*SETMODIFIERS ) ( int count, int mods[] );

enum MouseButton
{
	M_NONE = 0,
	M_LEFT = 1,
	M_RIGHT = 2
};

typedef struct
{
	/* A rectangle representing the edge to snap to */
	RECT rect;

	/* The co-ordinate to snap to when snapping
	(will either be an X or Y co-ordinate depending on the type of snap) */
	LONG snap;
} snap_t;

typedef struct
{
	snap_t snap_list[ MAX_SNAPS ];
	unsigned size;
	unsigned next;
} snap_info_t;

extern int                style;
extern snap_info_t x_snaps;
extern snap_info_t y_snaps;
extern HWND            hWnd;
extern RECT border;

extern unsigned MonitorSnapDistance;
extern unsigned WindowSnapDistance;


#endif // __WMMOUSEDLL_H__