#if !defined SNAP_HPP
#define      SNAP_HPP

#include <Windows.h>

LONG YPosIncSnap( POINT ptRel, LONG width, LONG height, bool top, bool bottom );
LONG XPosIncSnap( POINT ptRel, LONG width, LONG height, bool top, bool bottom );
void UpdateSnaps();

#endif