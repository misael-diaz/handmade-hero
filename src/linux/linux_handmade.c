#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <stdbool.h>
#include <string.h>
#include "handmade.h"

static bool X11Error;

static int linux_X11ErrorHandler(Display *display, XErrorEvent *ev)
{
	char errmsg[256];
	XGetErrorText(display, ev->error_code, errmsg, sizeof(errmsg));
	fprintf(stderr, "%s\n", errmsg);
	X11Error = true;
	return 0;
}

int main()
{
	fprintf(stdout, "%s", "Linux - HandMade Hero\n");
	Display *display = XOpenDisplay(NULL);
	if (!display) {
		fprintf(stdout, "%s", "XErrorOpenDisplay\n");
		exit(EXIT_FAILURE);
	}
	XSetErrorHandler(linux_X11ErrorHandler);
	Window root = DefaultRootWindow(display);
	Screen *screen = DefaultScreenOfDisplay(display);
	int screeno = DefaultScreen(display);
	XSetWindowAttributes attributes = {};
	attributes.background_pixel = BlackPixel(display, screeno);
	attributes.event_mask = (
		ExposureMask |
		KeyPressMask |
		KeyReleaseMask
	);

	int const x = 0;
	int const y = 0;
	int const width = WidthOfScreen(screen);
	int const height = HeightOfScreen(screen);
	int const window_border_width = 0;
	int const screen_depth = DefaultDepthOfScreen(screen);
	Visual *visual = DefaultVisual(display, screeno);
	Window window = XCreateWindow(
		display,
		root,
		x,
		y,
		width,
		height,
		window_border_width,
		screen_depth,
		InputOutput,
		visual,
		CWBackPixel | CWEventMask,
		&attributes
	);

	if (X11Error) {
		fprintf(stderr, "%s", "Fatal Xlib Error\n");
		XCloseDisplay(display);
		display = NULL;
		exit(EXIT_FAILURE);
	}

	GC gc = DefaultGC(display, screeno);
	XEvent ev = {};
	XStoreName(display, window, "Handmade Hero");
	XMapWindow(display, window);
	XWindowEvent(display, window, ExposureMask, &ev);

	struct game_memory Memory = {};
	Memory.PermanentStorageSize = MegaBytes(64);
	Memory.PermanentStorage = malloc(Memory.PermanentStorageSize);
	if (!Memory.PermanentStorage) {
		fprintf(stderr, "%s", "error: failed to allocate the game permanent storage\n");
		XDestroyWindow(display, window);
		XCloseDisplay(display);
		display = NULL;
		exit(EXIT_FAILURE);
	}
	memset(Memory.PermanentStorage, 0, Memory.PermanentStorageSize);

	// TODO: allocate the bitmap
	struct game_offscreen_buffer Buffer = {};

	GameUpdate(&Memory, &Buffer);

	// TODO: refactor this into a function called linux_pause() (not pause() because unistd.h defines one)
	// we pause here so that we can try to resize the window and not exit right away
	char c = 0;
	fprintf(stdout, "%s", "press any key to exit the game\n");
	fread(&c, sizeof(c), 1, stdin);

	free(Memory.PermanentStorage);
	Memory.PermanentStorage = NULL;
	Memory.PermanentStorageSize = 0;
	XDestroyWindow(display, window);
	XCloseDisplay(display);
	return 0;
}
