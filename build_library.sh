#!/usr/bin/env sh
ruby tools/split_sh.rb graphics.h HAL
if [ -f "hal_graphics.c" ] && [ -f "hal_graphics.h" ]; then
  clang -c -DHAL_DEBUG -DHAL_USE_GPU -DHAL_HAS_METAL -x objective-c -fno-objc-arc hal_graphics.c -framework Cocoa -framework AppKit -framework Metal -framework MetalKit
  ar -rv build/libhal_graphics.a hal_graphics.o
  rm hal_graphics.c hal_graphics.h hal_graphics.o
else
  echo "Try again, lol"
fi
