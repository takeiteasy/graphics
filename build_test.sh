#!/usr/bin/env sh
clang -DHAL_DEBUG -DHAL_USE_GPU -DHAL_HAS_METAL -x objective-c -fno-objc-arc graphics.h -framework Cocoa -framework AppKit -framework Metal -framework MetalKit -o build/graphics
if [ -f "build/graphics" ]; then
  ./build/graphics
  rm build/graphics
else
  echo "Try again, lol"
fi
