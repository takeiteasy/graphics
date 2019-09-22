@ECHO OFF
pushd "%~dp0"
call "C:\\Program Files (x86)\\Microsoft Visual C++ Build Tools\\vcbuildtools.bat" amd64
popd
cd build
cl /EHsc /Zi /W4 /utf-8 -I "../include" ../test.c ../src/graphics.c /link /subsystem:console user32.lib gdi32.lib
cd ..
