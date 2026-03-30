#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/timex.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "handmade.h"

#define HANDMADE_SO "lib/handmade.so"
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

// NOTE: even for multiple monitors this yields the refresh-rate of the screen (where the screen is
//       an abstraction in xlib) so we don't need to probe for individual monitor properties; the
//       properties of the virtual screen is what we actually need.
static int LinuxGetDisplayRefreshRate(Display *display, Window window)
{
	XRRScreenConfiguration *conf = XRRGetScreenInfo(display, window);
	short rate = XRRConfigCurrentRate(conf);

	XRRScreenResources *resources = XRRGetScreenResources(display, window);
	fprintf(stdout, "display framerate: %d\n", rate);
	fprintf(stdout, "number of crtcs: %d\n", resources->ncrtc);
	// NOTE: We only display crtcs that have meaningful dimensions (non-zero); if we have non-zero xoffset
	//       or yoffset we know that we have at least one extra monitor. Either there's scarce info or
	//       not at all on this, this is why we bothered to add this note. Now we have a reliable way
	//       to tell if our game could be running in a machine with multiple monitors.
	for (int idx = 0; idx != resources->ncrtc; ++idx) {
		XID crtc = resources->crtcs[idx];
		XRRCrtcInfo *info = XRRGetCrtcInfo(display, resources, crtc);
		if (info->width && info->height) {
			fprintf(
				stdout,
				"xoffset: %d yoffset: %d width: %d height: %d\n",
				info->x,
				info->y,
				info->width,
				info->height
				);
		}
		XRRFreeCrtcInfo(info);
	}
	XRRFreeScreenResources(resources);
	XRRFreeScreenConfigInfo(conf);
	return rate;
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

// NOTE: just showing some info so that we know how the system is going handle leap seconds (insert/remove)
int LinuxGetNTPInfo()
{
	errno = 0;
	struct timex timexbuf = {};
	int rc = ntp_adjtime(&timexbuf);
	if (-1 == rc) {
		fprintf(stderr, "%s", "NTP error\n");
		if (errno) {
			fprintf(stderr, "%s\n", strerror(errno));
		}
		return EXIT_FAILURE;
	} else if (TIME_OK) {
		fprintf(stdout, "%s", "system clock synchronized\n");
	} else if (TIME_INS) {
		fprintf(stdout, "%s", "leap second to be added to system clock\n");
	} else if (TIME_DEL) {
		fprintf(stdout, "%s", "leap second to be deleted to system clock\n");
	} else if (TIME_OOP) {
		fprintf(stdout, "%s", "system clock adjustment in progress\n");
	} else if (TIME_WAIT) {
		fprintf(stdout, "%s", "system clock adjustment pending adjustment status clearing\n");
	} else if (TIME_ERROR) {
		fprintf(stdout, "%s", "system clock is not synchronized to a reliable server\n");
	}

	if (timexbuf.status & STA_NANO) {
		char resolution[] = "(nsec)";
		fprintf(stdout, "system clock offset %s: %ld\n", resolution, timexbuf.offset);
	} else {
		char resolution[] = "(usec)";
		fprintf(stdout, "system clock offset %s: %ld\n", resolution, timexbuf.offset);
	}
	fprintf(stdout, "system clock tick (usec): %ld\n", timexbuf.tick);
	fprintf(stdout, "system clock precision (usec): %ld\n", timexbuf.precision);
	return 0;
}

// NOTE: with this you can verify the resolution of a given clock at least in my machine the
//       resolution of the monotonic clock is 1 nanosecond (probably the same in other machines).
void LinuxGetClockInfo(
		clockid_t clock_id,
		struct timespec * const clock_resolution
) {
	errno = 0;
	int rc = clock_getres(clock_id, clock_resolution);
	if (-1 == rc) {
		fprintf(stderr, "%s", "system clock error\n");
		if (errno) {
			fprintf(stderr, "%s\n", strerror(errno));
		}
		exit(EXIT_FAILURE);
	}
	fprintf(
		stdout,
		"clock resolution sec: %ld nsec: %ld\n",
		clock_resolution->tv_sec,
		clock_resolution->tv_nsec
	);
}

// NOTE: integer division is slower that float arithmetic but we are favoring simplicity over performance
//       here because we are doing this to determine the time period that the CPU needs to sleep to match
//       the game frame rate. Later we can drop in the faster implementation to decrease the overhead.
void LinuxSetTimeSpec(
	struct timespec * const clock_time,
	long const nsec
) {
	clock_time->tv_sec  = (nsec / 1000000000);
	clock_time->tv_nsec = (nsec % 1000000000);
}

void LinuxSetDelayTime(
	struct timespec * const clock_target,
	struct timespec const * const clock_start,
	struct timespec const * const clock_delta
) {
	clock_target->tv_sec = (
		(clock_start->tv_sec + clock_delta->tv_sec) +
		((clock_start->tv_nsec + clock_delta->tv_nsec) / 1000000000)
	);
	clock_target->tv_nsec = (
		((clock_start->tv_nsec + clock_delta->tv_nsec) % 1000000000)
	);
}

void LinuxDiffTimeSpec(
	struct timespec * const clock_delta,
	struct timespec const * const clock_start,
	struct timespec const * const clock_end
) {
	long nsec_diff = 0;
	long const nsec_start = 1000000000 * clock_start->tv_sec + clock_start->tv_nsec;
	long const nsec_end   = 1000000000 *   clock_end->tv_sec +   clock_end->tv_nsec;
	if (nsec_end > nsec_start) {
		nsec_diff = (nsec_end - nsec_start);
	} else {
		nsec_diff = (nsec_start - nsec_end);
	}
	clock_delta->tv_sec  = (nsec_diff / 1000000000);
	clock_delta->tv_nsec = (nsec_diff % 1000000000);
}

void LinuxCSumTimeSpec(
	struct timespec * const clock_csum,
	struct timespec const * const clock_delta
) {
	long const sec = (
		 (clock_csum->tv_sec  + clock_delta->tv_sec) +
		((clock_csum->tv_nsec + clock_delta->tv_nsec) / 1000000000)
	);
	long const nsec = (
		((clock_csum->tv_nsec + clock_delta->tv_nsec) % 1000000000)
	);
	clock_csum->tv_sec = sec;
	clock_csum->tv_nsec = nsec;
}

void LinuxDelay(
	clockid_t clock_id,
	struct timespec const * const clock_target
) {
	int rc = 0;
	if (CLOCK_MONOTONIC != clock_id) {
		fprintf(stderr, "%s", "unsupported clock_id\n");
	}
	do {
		rc = clock_nanosleep(clock_id, TIMER_ABSTIME, clock_target, NULL);
		if (EFAULT == rc) {
			fprintf(stderr, "%s", "invalid address for request time 'clock_target'\n");
		} else if (EINVAL == rc) {
			if (CLOCK_MONOTONIC != clock_id) {
				fprintf(stderr, "%s", "unsupported clock_id\n");
			} else {
				fprintf(stderr, "%s", "invalid value for 'clock_target' timespec\n");
			}
		}
	} while (EINTR == rc);
}

internal struct game_controller_input *GetControllerStub(
        struct game_input * const Input,
        int const ControllerIndex
) {
	Assert(0 <= ControllerIndex);
	size_t const Index = ControllerIndex;
	Assert(Index < ArrayCount(Input->Controllers));
	return &Input->Controllers[Index];
}

internal void GameUpdateStub(
        struct game_input *Input,
        struct game_memory *Memory,
        struct game_offscreen_buffer *Buffer
) {
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
	void *lhandmade = dlopen(HANDMADE_SO, RTLD_NOW);
	if (!lhandmade) {
		fprintf(stderr, "%s", "fail to open shared object handmade hero " HANDMADE_SO "\n");
		exit(EXIT_FAILURE);
	}
	void *GetController = dlsym(lhandmade, "GetController");
	if (!GetController) {
		fprintf(stderr, "%s", "failed to load GetController()\n");
		dlclose(lhandmade);
		exit(EXIT_FAILURE);
	}
	void *GameUpdate = dlsym(lhandmade, "GameUpdate");
	if (!GameUpdate) {
		fprintf(stderr, "%s", "failed to load GameUpdate()\n");
		dlclose(lhandmade);
		exit(EXIT_FAILURE);
	}
	if (LinuxGetNTPInfo()) {
		dlclose(lhandmade);
		exit(EXIT_FAILURE);
	}
	clockid_t clockid = CLOCK_MONOTONIC;
	struct timespec clock_monotonic_res = {};
	LinuxGetClockInfo(clockid, &clock_monotonic_res);
	fprintf(stdout, "%s", "Linux - HandMade Hero\n");
	Display *display = XOpenDisplay(NULL);
	if (!display) {
		fprintf(stdout, "%s", "XErrorOpenDisplay\n");
		dlclose(lhandmade);
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
		dlclose(lhandmade);
		XCloseDisplay(display);
		display = NULL;
		exit(EXIT_FAILURE);
	}

	// NOTE: here we are assuming data to be layout in LE
	if (LSBFirst != ImageByteOrder(display)) {
		fprintf(stderr, "%s", "Fatal Unsupported Display Error\n");
		dlclose(lhandmade);
		XCloseDisplay(display);
		display = NULL;
		exit(EXIT_FAILURE);
	}

	int const ScreenRefreshRate = LinuxGetDisplayRefreshRate(display, window);
	int const GameRateFPS = ScreenRefreshRate / 2;
	float const ElapsedTimePerFrameNanoSec = 1.0e9 / ((float) GameRateFPS);
	fprintf(stdout, "target elapsed time per frame %f\n", ElapsedTimePerFrameNanoSec);

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
		dlclose(lhandmade);
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
	long frames = 0;
	struct timespec TimeStart = {};
	struct timespec TimeEnd = {};
	struct timespec TimeDelta = {};
	struct timespec TimeSum = {};
	struct timespec ClockFrameDuration = {};

	struct game_controller_input *(*GetControllerFP)(
			struct game_input * const Input,
			int const ControllerIndex) = GetController;

	void (*GameUpdateFP)(
        struct game_input *Input,
        struct game_memory *Memory,
        struct game_offscreen_buffer *Buffer) = GameUpdate;

	LinuxSetTimeSpec(&ClockFrameDuration, ElapsedTimePerFrameNanoSec);
	while (Running) {
		clock_gettime(CLOCK_MONOTONIC_RAW, &TimeStart);
		struct timespec ClockLastTime = {};
		struct timespec ClockTargetTime = {};
		clock_gettime(clockid, &ClockLastTime);
		LinuxSetDelayTime(&ClockTargetTime, &ClockLastTime, &ClockFrameDuration);

		struct game_controller_input *NewKeyboardController = GetControllerFP(NewInput, 0);
		struct game_controller_input *OldKeyboardController = GetControllerFP(OldInput, 0);
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
		// TODO For analog input you can add (1<<15) to the input so that one
		//      does not need to use conditionals to normalize the input to the
		//      [-1, 1) range. This is assuming that the lowest value is
		//      -(1<<15), so if it's different (doubt it) you can easily apply
		//      this simple tactic for normalization.
		LinuxProcessPendingMessages(display, NewKeyboardController);

		// TODO to move the swapping to a pointer Swap() function
		NewInputIdx ^= 1;
		OldInputIdx ^= 1;
		NewInput = &Input[NewInputIdx];
		OldInput = &Input[OldInputIdx];

		LinuxDelay(clockid, &ClockTargetTime);
		// NOTE: since we swapped the inputs ahead of time for timing purposes we pass the
		//       OldInput because it actually refers to the current input for this frame
		GameUpdateFP(OldInput, &Memory, &Buffer);
		clock_gettime(CLOCK_MONOTONIC_RAW, &TimeEnd);
		LinuxDiffTimeSpec(&TimeDelta, &TimeStart, &TimeEnd);
		LinuxCSumTimeSpec(&TimeSum, &TimeDelta);
		++frames;
	}

	float OverallFPS = frames / (((float) TimeSum.tv_sec) + 1.0e-9 * ((float) TimeSum.tv_nsec));
	fprintf(stdout, "average fps: %.1f\n", OverallFPS);

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
	dlclose(lhandmade);
	return 0;
}
