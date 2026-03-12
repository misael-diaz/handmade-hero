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

HANDMADE_H = ../../include/handmade.h
HANDMADE_C = handmade.c
HANDMADE_O = ../../obj/handmade.o

all: $(HANDMADE_O) 

$(HANDMADE_O): $(HANDMADE_C) $(HANDMADE_H)
	$(CC) $(CCOPT) -I ../../include -c $(HANDMADE_C) -o $(HANDMADE_O)
clean:
	rm -f $(HANDMADE_O)
