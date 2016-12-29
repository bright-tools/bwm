#include "wmmousedll.h"
#include "Config.hpp"

#include <windows.h>

// The data in this segment is shared between all the instances of the DLL
#pragma data_seg ( "SHAREDDATA" )
unsigned MonitorSnapDistance = 40;

unsigned WindowSnapDistance = 40;

DWORD mod_count = 2;

int modifiers[ MAX_NUM_MODIFIER_KEYS ] = { VK_MENU, VK_CONTROL };

enum MouseButton MButtonMove = M_LEFT;
enum MouseButton MButtonResize = M_RIGHT;
enum MouseButton MButtonMinimise = M_LEFT;
enum MouseButton MButtonMaximise = M_RIGHT;
#pragma data_seg ()

#pragma comment(linker, "/section:SHAREDDATA,rws")

DLL_EXPORT void SetScreenSnap( int val )
{
	MonitorSnapDistance = val;
}

DLL_EXPORT void SetWindowSnap( int val )
{
	WindowSnapDistance = val;
}

DLL_EXPORT void SetMoveMButton( int val )
{
	MButtonMove = ( enum MouseButton )val;
}

DLL_EXPORT void SetResizeMButton( int val )
{
	MButtonResize = ( enum MouseButton )val;
}

DLL_EXPORT void SetMinimiseMButton( int val )
{
	MButtonMinimise = ( enum MouseButton )val;
}

DLL_EXPORT void SetMaximiseMButton( int val )
{
	MButtonMaximise = ( enum MouseButton )val;
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
