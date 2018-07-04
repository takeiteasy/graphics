#!/bin/sh
clang -x objective-c -fno-objc-arc test.c ../graphics.c ../extra/*.c -DGRAPHICS_ENABLE_OPENGL -framework Cocoa -framework OpenGL -I../ -I../extra
./a.out
rm a.out
