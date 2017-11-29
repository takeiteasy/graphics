#/bin/sh
clang -c -x objective-c -fmodules -fno-objc-arc app.c
ar -rv libapp.a app.o
rm app.o
clang test.c libapp.a
