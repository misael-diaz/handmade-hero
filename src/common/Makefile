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
HANDMADE_SO = ../../lib/handmade.so

all: $(HANDMADE_SO)

$(HANDMADE_SO): $(HANDMADE_C) $(HANDMADE_H)
	$(CC) $(CCOPT) -I ../../include -fpic $(HANDMADE_C) -o $(HANDMADE_SO) -shared
clean:
	rm -f $(HANDMADE_SO)
