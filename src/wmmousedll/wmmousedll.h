#ifndef __WMMOUSEDLL_H__
#define __WMMOUSEDLL_H__

#pragma once

typedef void (*SETKBHOOK) ();
typedef void (*REMOVEKBHOOK) ();
typedef int  (*GETINSTANCECOUNT) ();
typedef void (*SETMODIFIERS ) ( int count, int mods[] );

#endif // __WMMOUSEDLL_H__