@ECHO OFF
cl -I "include" test.c src\graphics.c /link /subsystem:windows user32.lib /entry:mainCRTStartup /utf-8 /OUT:build/test.exe
