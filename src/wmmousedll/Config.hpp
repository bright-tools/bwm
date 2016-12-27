/*
File: Config.hpp
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

#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#pragma once

#include <windows.h>

/// <summary>
/// Max number of simultaneous modifier keys that are supported
///  e.g. CTRL + ALT = 2 keys
/// </summary>
#define MAX_NUM_MODIFIER_KEYS 10U

// Snap margin between the window edge and the edge of the screen
extern unsigned MonitorSnapDistance;
// Snap margin between the window edge and the edge of other windows
extern unsigned WindowSnapDistance;

extern enum MouseButton MButtonMove;
extern enum MouseButton MButtonResize;
extern enum MouseButton MButtonMinimise;
extern enum MouseButton MButtonMaximise;

extern DWORD mod_count;

// The currently configured keyboard modifiers
extern int modifiers[ MAX_NUM_MODIFIER_KEYS ];

#endif