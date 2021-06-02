#!/usr/bin/env sh
emcc -O2 -g4 -DDEBUG -DGRAPHICS_EMCC_HTML -I./graphics test.c graphics/graphics.c -o build/index.js --preload-file tests/bmp/g/rgb32.bmp
