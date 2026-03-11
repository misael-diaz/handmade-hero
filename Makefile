#!/usr/bin/make
#
# handmade-hero				March 11, 2026
#
# source: Makefile
# author: @misael-diaz
#
# Synopsis:
# Defines the Makefile for building the program with GNU make.
#
# Copyright (c) 2025 Misael Diaz-Maldonado
#
# This file is released under the MIT License.
#

include make-inc

export CC
export CCOPT
export LIBS
export PLATFORM 

all: game

game:
	@$(MAKE) -C src
clean:
	@$(MAKE) -C src clean
