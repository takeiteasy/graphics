#/bin/sh
clang -c -x objective-c -fmodules -fno-objc-arc app.c
ar -rv libapp.a app.o
rm app.o
clang++ -std=c++1z test.cc libapp.a
