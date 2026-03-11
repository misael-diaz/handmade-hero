#!/usr/bin/make
#
# handmade-hero				March 11, 2026
#
# source: src/win32/Makefile
# author: @misael-diaz
#
# Synopsis:
# Defines the Makefile for building the program with GNU make.
#
# Copyright (c) 2025 Misael Diaz-Maldonado
#
# This file is released under the MIT License.
#

GAME = ../../exe/win32-handmade.exe
MAIN_C = win32_handmade.c
MAIN_O = ../../obj/win32_handmade.o

all: $(GAME)

$(GAME): $(MAIN_O)
	$(CC) $(COPTS) $(MAIN_O) -o $(GAME) -Wl,--subsystem,windows $(LIBS) -lgdi32

$(MAIN_O): $(MAIN_C)
	$(CC) $(COPTS) -c $(MAIN_C) -o $(MAIN_O)
clean:
	rm -f $(MAIN_O) $(GAME)
