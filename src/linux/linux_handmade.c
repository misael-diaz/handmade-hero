#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
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

	// NOTE: here we are assuming data to be layout in LE
	if (LSBFirst != ImageByteOrder(display)) {
		fprintf(stderr, "%s", "Fatal Unsupported Display Error\n");
		XCloseDisplay(display);
		display = NULL;
		exit(EXIT_FAILURE);
	}

	int nvisuals = 0;
	XVisualInfo template = {};
	template.visualid = XVisualIDFromVisual(visual);
	XVisualInfo *visinfo = XGetVisualInfo(
		display,
		VisualIDMask,
		&template,
		&nvisuals
	);

	fprintf(stdout, "bitmap-pad %d\n", BitmapPad(display));
	fprintf(stdout, "depth %d\n", visinfo->depth);
	fprintf(stdout, "red-mask 0x%lx\n", visual->red_mask);
	fprintf(stdout, "green-mask 0x%lx\n", visual->green_mask);
	fprintf(stdout, "blue-mask 0x%lx\n", visual->blue_mask);

	long unsigned red_shift = 0;
	long unsigned green_shift = 0;
	long unsigned blue_shift = 0;

	const long unsigned rgb_mask = 0xff;
	while ((rgb_mask << red_shift) != visual->red_mask) {
		red_shift += 8LU;
	}

	while ((rgb_mask << green_shift) != visual->green_mask) {
		green_shift += 8LU;
	}

	while ((rgb_mask << blue_shift) != visual->blue_mask) {
		blue_shift += 8LU;
	}

	fprintf(stdout, "red-shift %ld\n", red_shift);
	fprintf(stdout, "green-shift %ld\n", green_shift);
	fprintf(stdout, "blue-shift %ld\n", blue_shift);

	GC gc = DefaultGC(display, screeno);

	XEvent ev = {};
	XStoreName(display, window, "Handmade Hero");
	XMapWindow(display, window);
	XWindowEvent(display, window, ExposureMask, &ev);

	struct game_memory Memory = {};
	Memory.PermanentStorageSize = MegaBytes(64);
	Memory.TransientStorageSize = GigaBytes(4);

	errno = 0;
	Memory.PermanentStorage = mmap(
		NULL,
		Memory.PermanentStorageSize + Memory.TransientStorageSize,
		PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE,
		-1,
		0
	);
	if (!Memory.PermanentStorage || (((void*)-1) == Memory.PermanentStorage)) {
		fprintf(stderr, "%s", "error: failed to allocate the game storage\n");
		if (errno) {
			fprintf(stderr, "%s\n", strerror(errno));
		}
		XFree(visinfo);
		XDestroyWindow(display, window);
		XCloseDisplay(display);
		display = NULL;
		exit(EXIT_FAILURE);
	}

	Memory.TransientStorage = Memory.PermanentStorage + Memory.PermanentStorageSize;

	uint8_t const red = 0;
	uint8_t const green = 0xff;
	uint8_t const blue = 0;
	size_t const pixels = width * height;
	size_t const framesz = pixels * 4;
	int *framebuffer = Memory.TransientStorage;
	for (long unsigned i = 0; i != pixels; ++i) {
		framebuffer[i] = (
			(red << red_shift) + (green << green_shift) + (blue << blue_shift)
		);
	}

	char *data = Memory.TransientStorage;
	XImage *image = XCreateImage(
		display,
		visual,
		visinfo->depth,
		ZPixmap,
		0,
		data,
		width,
		height,
		32,
		0
	);

	XPutImage(display, window, gc, image, 0, 0, 0, 0, width, height);

	// TODO: allocate the bitmap
	struct game_offscreen_buffer Buffer = {};

	GameUpdate(&Memory, &Buffer);

	// TODO: refactor this into a function called linux_pause() (not pause() because unistd.h defines one)
	// we pause here so that we can try to resize the window and not exit right away
	char c = 0;
	fprintf(stdout, "%s", "press any key to exit the game\n");
	fread(&c, sizeof(c), 1, stdin);

	// NOTE: we have to nullify because Xlib will try to free the data but this is
	//       going to cause a problem because the data is not heap allocated so the
	//       right thing here is to nullify so that Xlib won't attempt to free a
	//       memory region that has not been allocated. We have checked this with
	//       valgrind.
	image->data = NULL;
	XDestroyImage(image);
	XFree(visinfo);
	XDestroyWindow(display, window);
	XCloseDisplay(display);
	return 0;
}
