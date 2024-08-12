![default](https://github.com/ndr3www/game-of-life/assets/164680506/e554f5ff-af9d-43b2-86fc-b9932d9717ac) ![mouse](https://github.com/user-attachments/assets/6dc48ef0-d5cc-4330-8f48-56eea50a2027)


# Installation

## Linux
Download the Linux package from [latest release](https://github.com/ndr3www/game-of-life/releases/latest), unzip and install these dependencies through your distribution's package manager:
`SDL2` `SDL2_gfx` `SDL2_ttf`

## Windows
Just download the Windows package from [latest release](https://github.com/ndr3www/game-of-life/releases/latest), unzip and you're good to go!

# Building from source

First, you need to install following dependencies:

`gcc` `SDL2` `SDL2_gfx` `SDL2_ttf`

then just:

```bash
git clone https://github.com/ndr3www/game-of-life
cd game-of-life
mkdir build
cd build
```
## Natively
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
## With MinGW-w64 toolchain
```bash
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../cmake/mingw.cmake ..
make
```
**Note:** The path to MinGW environment and the name of the compiler in `mingw.cmake` may differ on your system, so make sure to change them accordingly, if that's the case
# Usage
- Control the speed of simulation with arrow keys: **Left** - slow down, **Right** - speed up
- Change specific cell(s) state by point-and-click: **Left button** - alive, **Others** - dead
- Enable/disable auxiliary grid with **E**
- Pause/unpause by clicking **P**
- Clear the board from alive cells with **C**
- Restart the entire simulation by hitting **R**
- You can exit the application with **Escape**
