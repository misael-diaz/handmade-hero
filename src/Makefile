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

all: shared $(TARGET)

ifeq ($(PLATFORM),linux)
shared:
	@mkdir -p ../obj
	@mkdir -p ../lib
	@mkdir -p ../bin
	@$(MAKE) -C common
else
	@$(MAKE) -C common
endif

platform_win32: shared
	@$(MAKE) -C win32
platform_linux: shared
	@$(MAKE) -C linux

ifeq ($(PLATFORM),linux)
clean:
	@$(MAKE) -C linux clean
	@$(MAKE) -C common clean
else
clean:
	@$(MAKE) -C win32 clean
	@$(MAKE) -C common clean
endif
