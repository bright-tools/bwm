Bright Window Manager (BWM)
===========================

What's it For?
--------------

BWM's purpose is to speed up the task of moving windows around by allowing
you to perform operations like moving & re-sizing without needing to click on
the titlebar or frame of the window (you hold down shortcut keys and then click
anywhere in the Window).

What does it Do?
----------------

Window manager (WM) extensions for Windows:
  * Left-click and drag (anywhere in the window, not just the titlebar) with keyboard modifiers held down to move a window
  * Right-click and drag (anywhere in the window, not just the frame) with keyboard modifiers held down to resize a window
  * Double-left click with keyboard modifiers held down to maximise/restore a window
  * Double-right click with keyboard modifiers held down to minimise a window
  * Automatic "snapping" to the edges of the screen and other windows when moving or resizing

Keyboard modifers (and mouse button bindings) can be configured by selecting "Options" after right-clicking
on the notification icon in the task area.

Moving Windows
--------------

![movedemo](https://raw.githubusercontent.com/bright-tools/bwm/master/assets/move_demo.gif)

Resizing Windows
----------------

![resizedemo](https://raw.githubusercontent.com/bright-tools/bwm/master/assets/resize_demo.gif)

Maximising/Restoring/Minimising Windows
---------------------------------------

![maximisedemo](https://raw.githubusercontent.com/bright-tools/bwm/master/assets/maximise_demo.gif)

Dependencies
============

You'll need
  * [.NET 4.5](https://www.microsoft.com/en-gb/download/details.aspx?id=30653)
  * [Visual C Runtime](https://www.microsoft.com/en-gb/download/details.aspx?id=48145)

The installer should tell you about these, but I've found that it's not as
reliable as would be desired.

History
=======

This project was initially based on Markus Rollmann's project on CodeProject [1] - full
credit to Markus for his work.

Development
===========

You'll need
  * [Microsoft Visual Studio 2015](https://www.microsoft.com/en-us/download/details.aspx?id=48146)
  * [Microsoft Visual Studio 2015 Installer Projects](https://marketplace.visualstudio.com/items?itemName=VisualStudioProductTeam.MicrosoftVisualStudio2015InstallerProjects)

You can find an overview of how the application works [here](how_it_works.md).

References
==========

  * Original Project : http://www.codeproject.com/Articles/2706/X-Window-Manager-like-dragging-and-resizing-of-win 
