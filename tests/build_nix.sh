#!/bin/sh
clang test.c ../graphics.c -DGRAPHICS_ENABLE_OPENGL -lX11 -lm -ldl -lGL
./a.out
rm a.out
