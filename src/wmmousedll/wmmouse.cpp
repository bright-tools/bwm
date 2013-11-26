#include "wmmouse.h"

HINSTANCE			WMMouse::mMouseDllHinst		= 0;
SETKBHOOK			WMMouse::mSetMouseHook		= 0;
REMOVEKBHOOK		WMMouse::mRemoveMouseHook	= 0;
GETINSTANCECOUNT	WMMouse::mGetInstanceCount	= 0;

bool WMMouse::Init()
{
	if (mMouseDllHinst) return true;

	while (true)
	{
		// try to load wmmouse.dll
		if (! (mMouseDllHinst = LoadLibrary("wmmouse.dll"))) 
			break;

		// check number of instances
		if (! (mGetInstanceCount = (GETINSTANCECOUNT)GetProcAddress(mMouseDllHinst, "GetInstanceCount")))
			break;

		// get our hook function
		if (! (mSetMouseHook = (SETKBHOOK)GetProcAddress(mMouseDllHinst, "SetMouseHook")))
			break;

		// get our unkook function
		if (! (mRemoveMouseHook = (REMOVEKBHOOK)GetProcAddress(mMouseDllHinst, "RemoveMouseHook")))
			break;

		// report success
		return true;
	}

	// if we got here something went wrong
	if (mMouseDllHinst)
	{
		FreeLibrary(mMouseDllHinst);
		mMouseDllHinst = 0;
	}

	return false;
}
