#if !defined MOUSEHISTORY_HPP
#define      MOUSEHISTORY_HPP

#include "wmmousedll.h"
#include <windows.h>

class MouseHistory
{
private:
	enum MouseButton button;
	DWORD time;
	POINT position;

protected:
	bool InTime( DWORD p_time );

	bool InBounds( POINT pt );

public:
	MouseHistory();

	bool IsDoubleClick( const enum MouseButton p_button, const MOUSEHOOKSTRUCT p_ms );
};

#endif