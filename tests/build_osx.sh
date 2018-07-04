#!/bin/sh
clang -x objective-c -fno-objc-arc test.c ../graphics.c ../extra/*.c -DGRAPHICS_ENABLE_METAL -framework Cocoa -framework Metal -framework MetalKit -I../ -I../extra
./a.out
rm a.out
