#!/bin/sh
clang test.c ../graphics.c -DSGL_ENABLE_OPENGL -DSGL_ENABLE_JOYSTICKS -DSGL_ENABLE_STB_IMAGE -DSGL_ENABLE_BDF -lX11 -lm -ldl -lGL -lpthread
./a.out
rm a.out
