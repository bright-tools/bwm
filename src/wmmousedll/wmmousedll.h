#ifndef __WMMOUSEDLL_H__
#define __WMMOUSEDLL_H__

#pragma once

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

#endif // __WMMOUSEDLL_H__