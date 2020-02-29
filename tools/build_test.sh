#!/usr/bin/env sh
clang -DHAL_DEBUG -x objective-c -fno-common -fno-objc-arc examples/test.c -framework Cocoa -framework AppKit -framework Metal -framework MetalKit -o build/test
if [ -f "build/test" ]; then
  ./build/test
else
  echo "Try again, lol"
fi
