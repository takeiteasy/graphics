#!/bin/sh
clang -c graphics.c
ar -rv libgraphics.a graphics.o
rm graphics.o
clang test.c libgraphics.a -lX11
./a.out
rm a.out libgraphics.a
