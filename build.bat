@echo off

gcc src/handmade.c -o obj/handmade.exe -Wl,--subsystem,windows -lgdi32
