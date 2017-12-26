#!/bin/sh
clang -c -x objective-c -fmodules -fno-objc-arc graphics.c
ar -rv libgraphics.a graphics.o
rm graphics.o
clang main.c libgraphics.a
./a.out
rm a.out libgraphics.a
