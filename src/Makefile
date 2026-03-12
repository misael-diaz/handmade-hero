#!/usr/bin/make
#
# handmade-hero				March 11, 2026
#
# source: src/Makefile
# author: @misael-diaz
#
# Synopsis:
# Defines the Makefile for building the program with GNU make.
#
# Copyright (c) 2025 Misael Diaz-Maldonado
#
# This file is released under the MIT License.
#

TARGET = platform_$(PLATFORM)

all: $(TARGET)

platform_win32:
	@$(MAKE) -C win32
platform_linux:
	@mkdir -p ../obj && mkdir -p ../bin
	@$(MAKE) -C linux

ifeq ($(PLATFORM),linux)
clean:
	@$(MAKE) -C linux clean
else
clean:
	@$(MAKE) -C win32 clean
endif
