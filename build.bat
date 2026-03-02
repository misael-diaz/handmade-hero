@echo off
set PATH=%PATH%;C:\MinGW\bin
gcc src/handmade.c -o obj/handmade.exe -Wl,--subsystem,windows -lgdi32
