#!/usr/bin/env sh
clang -DHAL_STATIC -DHAL_DEBUG -DHAL_USE_GPU -DHAL_HAS_METAL -x objective-c -fno-common -fno-objc-arc examples/test.c -framework Cocoa -framework AppKit -framework Metal -framework MetalKit -o build/test
if [ -f "build/test" ]; then
  ./build/test
else
  echo "Try again, lol"
fi
