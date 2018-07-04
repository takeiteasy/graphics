# graphics.h

Cross platform 2D software rendering graphics library, inspired by [graphics.h](https://web.stanford.edu/class/archive/cs/cs106b/cs106b.1126/materials/cppdoc/graphics.html) and [minifb](https://github.com/emoon/minifb). Also taking inspiration from [SDL 1.2](https://www.libsdl.org/) and [QuickCG](http://lodev.org/cgtutor/)


### Supported

- Windows, OSX and Linux (X11)
- Keyboard and mouse events
- Primitive shapes
- BMP loading (8, 24 & 32 bpp)
- Save surfaces to BMP file
- Text rendering (adapted from [dhepper/font8x8](https://github.com/dhepper/font8x8))
- Optional OpenGL & Metal backends (Just for rendering to screen)
- Optional extras (BDF rendering, stb_image to surface)

See below screenshots for TODO list and the examples folder for some idea of how to use. Still a WIP, API subject to change a lot.


### Building

No external libraries are used unless you want to use the alternate backends (enable them by defining GRAPHICS_ENABLE_(OPENGL/METAL)).

On OSX you'll have to link Cocoa framework and include the ```-x objective-c -fno-objc-arc``` flags.

On Linux use the ```-lX11 -lm``` flags, and if you're using the OpenGL backend include ```-ld```.

I've never even bothered trying to build stuff on Windows outside of Visual Studio, so I can't help you there, but I don't think it matters.

One note for using this on Linux is that when rendering to the framebuffer, X11 can't automatically strech stuff like ```StretchDIBits``` and ```CGContextDrawImage``` can - so resizing the window is disabled (until I can find a solution) unless you're using the OpenGL backend.


### Screenshots

<p align="center">
  <img src="https://raw.githubusercontent.com/takeiteasy/graphics.h/master/screenshots/screenshot_osx.png">
</p>

<p align="center">
  <img src="https://raw.githubusercontent.com/takeiteasy/graphics.h/master/screenshots/screenshot_win.png">
</p>

<p align="center">
  <img src="https://raw.githubusercontent.com/takeiteasy/graphics.h/master/screenshots/screenshot_nix.png">
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

### MAYBE TODO

- RGBA surfaces (instead of just RGB?)
- Vulkan/DirectX/~~Metal~~ backends
- Documentation & comments
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
