# graphics.h

Cross platform software rendering library, inspired by [SDL 1.2](https://www.libsdl.org/), [graphics.h](https://web.stanford.edu/class/archive/cs/cs106b/cs106b.1126/materials/cppdoc/graphics.html), [minifb](https://github.com/emoon/minifb) and [QuickCG](http://lodev.org/cgtutor/). It was initially forked from [minifb](https://github.com/emoon/minifb), but has evolved beyond.

Designed to be a drop-in and use with little hassle sort of deal. I don't know if anyone will really find a use for this besides me. But it's very fun to work on. It's not explicitly a game development library, but that'll be the most likely one. It's most likely less efficient, slower, more buggy and has less features, support, compatibility and portability than SDL - Just so you know, if you didn't already guess.


## Features

- OSX (Carbon, OpenGL, Metal), Windows (GDI, OpenGL, DirectX 9), Linux (X11, OpenGL) (so far, see project page for planned stuff).
- Multiple Windows
- Keyboard, mouse and window events.
- Text rendering via in-built font (adapted from [dhepper/font8x8](https://github.com/dhepper/font8x8)) or BDF files
- BMP (24 or 32 bpp uncompressed)
- Message box & dialog support (adapted from [AndrewBelt/osdialog](https://github.com/AndrewBelt/osdialog))


See below screenshots, and check out the examples folder for some idea of how to use. ___Still a WIP, API subject to change a lot___. For documentation, visit [the docs page](https://takeiteasy.github.io/hal/).  Also, see the projects tab for an idea of progress, planned features and ideas.

OS X will be the primary platform for all stuff, then after that is working to a level I like I will finish the windows port and after that the linux port. It's too much effort changing something on OS X then doing it on other platforms all the time. __At the moment Windows & Linux won't build__ (because I removed them).


## Building

No external libraries are used unless you want to use the alternate backends. I don't know how to make CMake files yet so backends will have to be defined manually. You can enable or disable features using macros, there will be a list when everything is closer to being done.

On OS X you'll have to link Cocoa framework ```-framework Cocoa``` and include the ```-x objective-c -fno-objc-arc``` flags. If you're using OpenGL backend ```-framework OpenGL``` and ```-framework Metal -framework MetalKit``` for Metal. Also, if you're using XCode you'll also have to disable modules in the build settings, becuase modules imports curses with stdio for some reason.

**NOTE**: On OS X 10.14, something changed and CoreGraphics isn't working like it used to. So if you're using 10.14 Metal is now the default rending backend. See above.

On Linux you'll have to link libX11 and libm ```-lX11 -lm```, and if you're using the OpenGL backend include ```-ld -lGL```. **NOTE**: X11 can't automatically strech stuff being rendered like ```StretchDIBits``` and ```CGContextDrawImage``` can - so resizing the window is disabled (_until I can find a solution_) unless you're using the OpenGL backend.

On Windows (Visual Studio) you'll have to add ```/utf-8``` to the command line options or unicode decoding won't work properly. I don't know why, but it doesn't.

**Tested on** (so far):
- OS X 10.12, 10.13 & 10.14 (clang)
- Windows 7 x64 (MSVC)
-  Ubuntu 19 x86_64 (Linux 4.13.0-16) (Inside VM only) (clang)


## Screenshot

<p align="center">
  <img src="https://raw.githubusercontent.com/takeiteasy/graphics.h/master/screenshot.png">
</p>


## License

```Created by Rory B. Bellows on 26/11/2017.
Copyright © 2017-2019 George Watson. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
*   Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
*   Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
*   Neither the name of the <organization> nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL GEORGE WATSON BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
