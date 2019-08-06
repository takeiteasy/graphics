#!/usr/bin/env sh
emcc -O2 -g4 -DHAL_NO_ALERTS -I./include test.c src/graphics.c -o build/index.js --preload-file old/test/resources/Uncompressed-24.bmp
