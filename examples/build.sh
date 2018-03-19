#!/bin/sh
clang -x objective-c -fno-objc-arc $* ../graphics.c -framework Cocoa
./a.out
rm ./a.out
