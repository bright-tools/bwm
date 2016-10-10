/*
File: MouseHistory.cpp
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

#include "MouseHistory.hpp"

bool MouseHistory::InTime( DWORD p_time )
{
	return ( ( p_time - time ) <= GetDoubleClickTime() );
}

bool MouseHistory::InBounds( POINT pt )
{
	const int wd = GetSystemMetrics( SM_CXDOUBLECLK ) / 2;
	const int ht = GetSystemMetrics( SM_CYDOUBLECLK ) / 2;
	return ( ( ( pt.x >= ( position.x - wd ) ) && ( pt.x < ( position.x + wd ) ) ) && ( ( ( position.y >= ( pt.y - ht ) ) && ( position.y <= ( pt.y + ht ) ) ) ) );
}

MouseHistory::MouseHistory()
{
	button = M_NONE;
}

bool MouseHistory::IsDoubleClick( const enum MouseButton p_button, const MOUSEHOOKSTRUCT p_ms )
{
	const DWORD nowTime = GetTickCount();
	// perform check
	bool isDblClk = ( ( button == p_button ) && ( InTime( nowTime ) ) && ( InBounds( p_ms.pt ) ) );

	// update history
	button = p_button;
	time = nowTime;
	position = p_ms.pt;

	if( isDblClk )
	{
		button = M_NONE;
	}
	// return result
	return isDblClk;
}
