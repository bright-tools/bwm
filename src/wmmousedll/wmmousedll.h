/*
File: wmmousedll.h
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