# graphics.h

Cross platform 2D software rendering graphics library, inspired by [graphics.h](https://web.stanford.edu/class/archive/cs/cs106b/cs106b.1126/materials/cppdoc/graphics.html) and [minifb](https://github.com/emoon/minifb). Also taking inspiration from [SDL 1.2](https://www.libsdl.org/) and [QuickCG](http://lodev.org/cgtutor/)

### Supported

- Windows, Cocoa and Linux (X11)
- Keyboard and mouse events
- Primitive shapes
- BMP loading (8, 24 & 32 bpp)
- Save surfaces to BMP file
- Text rendering (adapted from [dhepper/font8x8](https://github.com/dhepper/font8x8))
- Optional OpenGL & Metal backends (Just for rendering to screen)

See below screenshots for TODO list and the examples folder for some idea of how to use. Still a WIP, API subject to change a lot.

### Screenshots

<p align="center">
  <img src="https://raw.githubusercontent.com/takeiteasy/graphics.h/master/screenshot_osx.png">
</p>

<p align="center">
  <img src="https://raw.githubusercontent.com/takeiteasy/graphics.h/master/screenshot_win.png">
</p>

<p align="center">
  <img src="https://raw.githubusercontent.com/takeiteasy/graphics.h/master/screenshot_nix.png">
</p>

## TODO

- Window flags
- Colour escapes for print()
- Cursor lock & hide
- Extended surface functions, ~~resize~~, rotate, filters, etc
- Joystick/Gamepad input
- Add fill option for ellipse_rotated
- Add rotated rect function
- Add line width option
- ~~Fix as many warnings as possible~~

### MAYBE TODO

- Vulkan/DirectX/~~Metal~~ backends
- Documentation & comments
- ~~OpenGL 1.X fallback for OSX? (Dynamic OpenGL loader for OSX?)~~ Unneeded
- Wayland/Mir window code
- C++ OOP wrapper
- libtcc interactive player (like [CToy](https://github.com/anael-seghezzi/CToy))
- More examples

## License

```Copyright (c) 2013, George Watson
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the <organization> nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL RUSTY SHACKLEFORD BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.```
