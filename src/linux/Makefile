#!/usr/bin/make
#
# handmade-hero				March 11, 2026
#
# source: src/linux/Makefile
# author: @misael-diaz
#
# Synopsis:
# Defines the Makefile for building the program with GNU make.
#
# Copyright (c) 2025 Misael Diaz-Maldonado
#
# This file is released under the MIT License.
#

GAME = ../../bin/linux-handmade.bin
MAIN_C = linux_handmade.c
MAIN_O = ../../obj/linux_handmade.o

all: $(GAME) 

$(GAME): $(MAIN_O)
	$(CC) $(COPTS) $(MAIN_O) -o $(GAME) $(LIBS) -lX11

$(MAIN_O): $(MAIN_C)
	$(CC) $(COPTS) -c $(MAIN_C) -o $(MAIN_O)
clean:
	rm -f $(MAIN_O) $(GAME)
