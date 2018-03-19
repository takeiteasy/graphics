#!/bin/sh
clang -x objective-c -fno-objc-arc test.c ../graphics.c -DGRAPHICS_EXTRA_COLOURS -DGRAPHICS_EXTRA_FONTS -DGRAPHICS_OPENGL_BACKEND -framework Cocoa -framework OpenGL -framework AppKit
./a.out
rm a.out
