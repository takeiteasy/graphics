#!/bin/sh
clang -x objective-c -fno-objc-arc test.c ../graphics.c -DSGL_ENABLE_FREETYPE -framework Cocoa -framework Metal -framework MetalKit -I../ -I../3rdparty
./a.out
rm a.out
