Bright Window Manager (BWM) - How It Works
==========================================

Overview
--------

The application is split into two largely independent components:

  * Configuration GUI
  * Worker DLL

Configuration GUI
-----------------

The entry point to the GUI is the icon in the task tray which allows the user to call up the configuration dialog or exit the application.  This component is implemented in C# using [WPF](https://msdn.microsoft.com/en-us/library/ms754130(v=vs.110).aspx)

The GUI interface loads the worker DLL & calls various methods in order to transfer the configuration.  From this point the DLL is operational without further interaction from the GUI component.

Worker DLL
----------

The worker DLL is written in C++ and hooks window messages by using [SetWindowsHookEx](https://msdn.microsoft.com/en-us/library/windows/desktop/ms644990(v=vs.85).aspx).  This means that the DLL is injected into the process owning each window.  In this way, window messages can be examined & the DLL can determine whether or not a move/resize operation needs to be performed, based on the current status of the modifier keys.
