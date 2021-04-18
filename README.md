Chocolate-Wolfenstein-3D
========================

![alt tag](screenshots/crt_aspect.png)

This is a fork of Fabien Sanglard's [Chocolate-Wolfenstein-3D](https://github.com/fabiensanglard/Chocolate-Wolfenstein-3D) which upgrades the codebase to use SDL2 and makes some
other small changes.

* Now using CMake for cross-platform builds.
* Replaces OpenGL code with the SDL2 renderer for CRT emulation.
* Support for modern game controllers (Xbox, PS3/PS4, etc) via the SDL2 GameController API.
    * When using a modern game controller, the game supports moving and strafing at the same time. 

This is still a WIP. Things I'm planning to do:
- [ ] Detailed instructions for how to build for each platform (via command line, and CLion IDE).
- [ ] Ports to other platforms/game consoles (PSP, PS Vita, etc).

## Build & Run

### Data Files

First copy your Wolf3D data files into the `data` directory. The files needed are:

```
AUDIOHED
AUDIOT
GAMEMAPS
MAPHEAD
VGADICT
VGAGRAPH
VGAHEAD
VSWAP
```

The data files can have the following extensions:

* .WL1 - Wolfenstein 3D shareware version (episode 1 only)
* .WL6 - Wolfenstein 3D full version (episode 1 to 6)
* .SOD - Wolfenstein 3D Spear of Destiny demo and full version
* .SD2 - Wolfenstein 3D Spear of Destiny Mission 2: Return to Danger
* .SD3 - Wolfenstein 3D Spear of Destiny Mission 3: Ultimate Challenge

You will need to ensure the game is configured for your version of the data files. You can do this by enabling specific
definitions in the `version.h` file, which has some comments with instructions about what to configure. By default the game is configured for the `.WL1` and `.WL6` data files.

If you don't have data files, here are some links:
* [Shareware files for Wolfenstein 3D](http://maniacsvault.net/ecwolf/files/shareware/wolf3d14.zip)
* [Shareware files for Spear of Destiny](http://maniacsvault.net/ecwolf/files/shareware/soddemo.zip)
* [Purchases the full game](https://www.gog.com/game/wolfenstein_3d_and_spear_of_destiny?pp=979014556ab8cdd13048702896fb99fcc40fe793)

### OSX

These instructions assume you have Homebrew installed. If you don't, please [install](https://brew.sh/) it.

Install all of the required dependencies:

```shell
brew update
brew install cmake sdl2 sdl2_mixer
```

Generate the build files:

```shell
mkdir build && cd build
cmake ..
```

Compile the game:

```shell
make -j4
```

Run the game:

```shell
./Chocolate-Wolfenstein-3D
```

### Windows

TODO!

### Linux

TODO!

## Play

### Command Line Arguments

**Option**|**Description**
:-----:|:-----:
--help|Show all command line options.
--windowed|Run the game in a window.
—-scale2x|Run the game at double the resolution.
--tedlevel <level>|Starts the game in the given level
--baby|Sets the difficulty to baby for tedlevel
--easy|Sets the difficulty to easy for tedlevel
--normal|Sets the difficulty to normal for tedlevel
--hard|Sets the difficulty to hard for tedlevel
--nowait|Skips intro screens
--windowed[-mouse]|Starts the game in a window [and grabs mouse]
--ignorenumchunks|Ignores the number of chunks in VGAHEAD.* (may be useful for some broken mods)
--configdir <dir>|Directory where config file and save games are stored (Windows default: current directory

### Game Controller Support

With the help of the SDL2 GameController API the game now supports modern controllers (Xbox 360, Xbox One, PS3/PS4, etc).

Here is the button layout for controllers. This is not currently customizable.

**Action**|**Button**
:-----:|:-----:
Move Forward/Backward|Left-Stick Up/Down
Strafe Left/Right|Left-Stick Left/Right
Look Left/Right|Right-Stick Left/Right
Use|A
Shoot|Right-Trigger or Right-Shoulder
Sprint|Left-Trigger or Left-Shoulder
Cycle Previous Weapon|Y
Cycle Next Weapon|B
Pause|Pause
Escape|Back

*Note: When using a controller, the game now supports moving and strafing at the same time.*

## Credits

* [Fabien Sanglard](https://fabiensanglard.net/) for original [Chocolate-Wolfenstein-3D](https://github.com/fabiensanglard/Chocolate-Wolfenstein-3D).
* Based on Wolf4SDL by Moritz "Ripper" Kroll (http://www.chaos-software.de.vu).
* Original Wolfenstein 3D by id Software (http://www.idsoftware.com).
