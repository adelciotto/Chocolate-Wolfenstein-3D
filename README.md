Chocolate-Wolfenstein-3D
========================

![alt tag](screenshots/crt_aspect.png)

This is a fork of Fabien Sanglard's [Chocolate-Wolfenstein-3D](https://github.com/fabiensanglard/Chocolate-Wolfenstein-3D) which upgrades the codebase to use SDL2 and makes some
other small changes.

* Now using CMake for cross-platform builds.
* Replaces OpenGL code with the SDL2 renderer for CRT emulation.

This is still a WIP. Things I'm planning to do:
- [ ] Detailed instructions for how to build for each platform (via command line, and CLion IDE).
- [ ] Replace Joystick API with the new GameController API.
- [ ] Ports to game consoles (PSP, PS Vita, etc).

## Credits

* [Fabien Sanglard](https://fabiensanglard.net/) for original [Chocolate-Wolfenstein-3D](https://github.com/fabiensanglard/Chocolate-Wolfenstein-3D).
* Based on Wolf4SDL by Moritz "Ripper" Kroll (http://www.chaos-software.de.vu).
* Original Wolfenstein 3D by id Software (http://www.idsoftware.com).
