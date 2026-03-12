#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <stdbool.h>

static bool X11Error;

static int linux_X11ErrorHandler(Display *display, XErrorEvent *ev)
{
	char errmsg[256];
	XGetErrorText(display, ev->error_code, errmsg, sizeof(errmsg));
	fprintf(stderr, "%s\n", errmsg);
	X11Error = true;
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

	// we pause here so that we can try to resize the window and not exit right away
	char c = 0;
	fprintf(stdout, "%s", "press any key to exit the game\n");
	fread(&c, sizeof(c), 1, stdin);

	XDestroyWindow(display, window);
	XCloseDisplay(display);
	return 0;
}
