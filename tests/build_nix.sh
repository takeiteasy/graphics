#!/bin/sh
clang test.c ../graphics.c -DGRAPHICS_EXTRA_COLOURS -DGRAPHICS_EXTRA_FONTS -lX11 -lm
