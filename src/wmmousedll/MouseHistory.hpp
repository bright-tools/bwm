/*
File: MouseHistory.hpp
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