#!/bin/sh
clang test.c ../graphics.c -DGRAPHICS_EXTRA_COLOURS -DGRAPHICS_EXTRA_FONTS -DGRAPHICS_OPENGL_BACKEND -lX11 -lm -ldl -lGL
