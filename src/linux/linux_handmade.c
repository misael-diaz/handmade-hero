#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "handmade.h"

#define KBD_UP XKeysymToKeycode(display, XK_Up)
#define KBD_DOWN XKeysymToKeycode(display, XK_Down)
#define KBD_LEFT XKeysymToKeycode(display, XK_Left)
#define KBD_RIGHT XKeysymToKeycode(display, XK_Right)
#define KBD_ESC XKeysymToKeycode(display, XK_Escape)

static bool Running;
static bool X11Error;

// STUDY: familiarize with the Linux kernel io_uring API for async io, we are going to use this as we advance
//        with the game-dev series
//
//        https://man7.org/linux/man-pages/man7/io_uring.7.html
//

static int LinuxX11ErrorHandler(Display *display, XErrorEvent *ev)
{
	char errmsg[256];
	XGetErrorText(display, ev->error_code, errmsg, sizeof(errmsg));
	fprintf(stderr, "%s\n", errmsg);
	X11Error = true;
	return 0;
}

static void LinuxProcessKeyboardInput(
	struct game_button_state * const NewState,
	bool const IsDown
) {
	Assert(IsDown != NewState->EndedDown);
	NewState->EndedDown = IsDown;
	++NewState->HalfTransitionCount;
}

static void LinuxProcessPendingMessages(
	Display * const display,
	struct game_controller_input * const KeyboardController
) {
	XEvent ev = {};
	// NOTE: Xlib does not store key transition states so we have to determine transition ourselves
	while (XPending(display)) {
		XNextEvent(display, &ev);
		if (KeyPress == ev.type) {
			bool const IsDown = true;
			if (KBD_LEFT == ev.xkey.keycode) {
				if (!KeyboardController->Left.WasDown) {
					LinuxProcessKeyboardInput(&KeyboardController->Left, IsDown);
					KeyboardController->Left.WasDown = true;
				}
			} else if (KBD_RIGHT == ev.xkey.keycode) {
				if (!KeyboardController->Right.WasDown) {
					LinuxProcessKeyboardInput(&KeyboardController->Right, IsDown);
					KeyboardController->Right.WasDown = true;
				}
			} else if (KBD_UP == ev.xkey.keycode) {
				if (!KeyboardController->Up.WasDown) {
					LinuxProcessKeyboardInput(&KeyboardController->Up, IsDown);
					KeyboardController->Up.WasDown = true;
				}
			} else if (KBD_DOWN == ev.xkey.keycode) {
				if (!KeyboardController->Down.WasDown) {
					LinuxProcessKeyboardInput(&KeyboardController->Down, IsDown);
					KeyboardController->Down.WasDown = true;
				}
			} else if (KBD_ESC == ev.xkey.keycode) {
				fprintf(stdout, "%s", "Quitting Game\n");
				Running = false;
				return;
			}
		} else if (KeyRelease == ev.type) {
			bool const IsDown = false;
			if (KBD_LEFT == ev.xkey.keycode) {
				if (KeyboardController->Left.WasDown) {
					LinuxProcessKeyboardInput(&KeyboardController->Left, IsDown);
				}
				KeyboardController->Left.WasDown = false;
			} else if (KBD_RIGHT == ev.xkey.keycode) {
				if (KeyboardController->Right.WasDown) {
					LinuxProcessKeyboardInput(&KeyboardController->Right, IsDown);
				}
				KeyboardController->Right.WasDown = false;
			} else if (KBD_UP == ev.xkey.keycode) {
				if (KeyboardController->Up.WasDown) {
					LinuxProcessKeyboardInput(&KeyboardController->Up, IsDown);
				}
				KeyboardController->Up.WasDown = false;
			} else if (KBD_DOWN == ev.xkey.keycode) {
				if (KeyboardController->Down.WasDown) {
					LinuxProcessKeyboardInput(&KeyboardController->Down, IsDown);
				}
				KeyboardController->Down.WasDown = false;
			}
		}
	}
}

// we map the entire file into mapped region and we can safely close the file descriptor; the caller
// should be at a better position to handle errors so we only log to the console what went wrong
DEBUG struct debug_read_file_result PlatformReadEntireFile(char const * const filename)
{
	struct debug_read_file_result res = {};
	errno = 0;
	int const fd = open(filename, O_RDONLY);
	if (-1 == fd) {
		fprintf(stderr, "failed to read file: %s", filename);
		if (errno) {
			fprintf(stderr, "%s\n", strerror(errno));
		}
		return res;
	}

	errno = 0;
	off_t const filesz = lseek(fd, 0, SEEK_END);
	if (-1 == filesz) {
		fprintf(stderr, "failed to determine the size of the file: %s", filename);
		close(fd);
		if (errno) {
			fprintf(stderr, "%s\n", strerror(errno));
		}
		return res;
	}

	errno = 0;
	if (-1 == lseek(fd, 0, SEEK_SET)) {
		fprintf(stderr, "failed to rewind file: %s", filename);
		close(fd);
		if (errno) {
			fprintf(stderr, "%s\n", strerror(errno));
		}
		return res;
	}

	errno = 0;
	void *data = mmap(NULL, filesz, PROT_READ, MAP_PRIVATE, fd, 0);
	if (!data || ((void*)-1) == data) {
		fprintf(stderr, "failed to read into memory the file: %s", filename);
		close(fd);
		if (errno) {
			fprintf(stderr, "%s\n", strerror(errno));
		}
		return res;
	}
	close(fd);
	res.Data = data;
	res.FileSize = filesz;
	return res;
}

// on GNU/Linux we don't need to do anything, the mapped region is going to be released when the app ends
DEBUG void PlatformFreeFile(void *buffer)
{
	return;
}

int main()
{
#if HANDMADE_DEV
	long const pagesz = sysconf(_SC_PAGESIZE);
	void * const BaseAddress = (void * const) ((pagesz * pagesz) * (pagesz / 2));
	int const MMapFlags = MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED;
#else
	void *BaseAddress = NULL;
	int const MMapFlags = MAP_ANONYMOUS | MAP_PRIVATE;
#endif
	fprintf(stdout, "%s", "Linux - HandMade Hero\n");
	Display *display = XOpenDisplay(NULL);
	if (!display) {
		fprintf(stdout, "%s", "XErrorOpenDisplay\n");
		exit(EXIT_FAILURE);
	}
	XSetErrorHandler(LinuxX11ErrorHandler);
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
		BaseAddress,
		Memory.PermanentStorageSize + Memory.TransientStorageSize,
		PROT_READ | PROT_WRITE,
		MMapFlags,
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
	memset(Memory.PermanentStorage, 0, Memory.PermanentStorageSize + Memory.TransientStorageSize);

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

	// NOTE: we assume that the code is going to be executed from the top-level of this project and this
	//       will go away or be replaced with a bitmap load depending on the next episodes of the series
	struct debug_read_file_result File = PlatformReadEntireFile("src/linux/"__FILE__);

	// TODO: allocate the bitmap
	struct game_offscreen_buffer Buffer = {};

	// NOTE: this is going to probably change based on the what to expect for the next episode, stream 17
	struct game_input Input[2] = {};
	int NewInputIdx = 0;
	int OldInputIdx = 1;
	struct game_input *NewInput = &Input[NewInputIdx];
	struct game_input *OldInput = &Input[OldInputIdx];

	Running = true;
	while (Running) {
		struct game_controller_input *NewKeyboardController = GetController(NewInput, 0);
		struct game_controller_input *OldKeyboardController = GetController(OldInput, 0);
		memset(NewKeyboardController, 0, sizeof(*NewKeyboardController));
		memset(NewKeyboardController, 0, sizeof(*NewKeyboardController));
		for (
			size_t ButtonIndex = 0;
			ButtonIndex != ArrayCount(NewKeyboardController->Buttons);
			++ButtonIndex) {
			bool const EndedDown = OldKeyboardController->Buttons[ButtonIndex].EndedDown;
			bool const WasDown = OldKeyboardController->Buttons[ButtonIndex].WasDown;
			NewKeyboardController->Buttons[ButtonIndex].EndedDown = EndedDown;
			NewKeyboardController->Buttons[ButtonIndex].WasDown = WasDown;
		}
		LinuxProcessPendingMessages(display, NewKeyboardController);
		GameUpdate(&Input[0], &Memory, &Buffer);
		// TODO to move the swapping to a pointer Swap() function
		NewInputIdx ^= 1;
		OldInputIdx ^= 1;
		NewInput = &Input[NewInputIdx];
		OldInput = &Input[OldInputIdx];
	}

	// TODO: refactor this into a function called LinuxPause() (not pause() because unistd.h defines one)
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
