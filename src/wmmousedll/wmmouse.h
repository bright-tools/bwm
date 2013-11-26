#ifndef __WMMOUSE_H__
#define __WMMOUSE_H__

#include <windows.h>
#include "wmmousedll.h"

// 
// a wrapper class to access the wmmousedll
//
class WMMouse
{
protected:
	static HINSTANCE			mMouseDllHinst;
	static SETKBHOOK			mSetMouseHook;
	static REMOVEKBHOOK			mRemoveMouseHook;
	static GETINSTANCECOUNT		mGetInstanceCount;

public:
	static bool Init();
	static void Done()
		{ if (mMouseDllHinst) FreeLibrary(mMouseDllHinst); }

	static void SetMouseHook()
		{ if (mMouseDllHinst) mSetMouseHook();	}

	static void RemoveMouseHook()
		{ if (mMouseDllHinst) mRemoveMouseHook(); }
		
	static int GetInstanceCount()
	{
		if (mMouseDllHinst)
			return mGetInstanceCount();
		else
			return 0;
	}
};

#endif // __WMMOUSE_H__