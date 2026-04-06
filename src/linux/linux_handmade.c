#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
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
	int64 const nsec
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
	int64 nsec_diff = 0;
	int64 const nsec_start = 1000000000 * clock_start->tv_sec + clock_start->tv_nsec;
	int64 const nsec_end   = 1000000000 *   clock_end->tv_sec +   clock_end->tv_nsec;
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
	int64 const sec = (
		 (clock_csum->tv_sec  + clock_delta->tv_sec) +
		((clock_csum->tv_nsec + clock_delta->tv_nsec) / 1000000000)
	);
	int64 const nsec = (
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
        uint32 const ControllerIndex
) {
	uint32 const Index = ControllerIndex;
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

// NOTE you are going to need a logger (it has become evident) but that will have to wait for now
// TODO some of the console loggin here matters for debugging mostly so make sure to remove them for
//      the production-grade build
internal void LinuxUpdateHandmadeLib(
	struct stat * const LastStatus,
	void ** const lhandmade,
	struct game_code * const GameCode
) {
	struct stat CurStatus = {};
	int rc = stat(HANDMADE_SO, &CurStatus);
	if (-1 == rc) {
		fprintf(stderr, "%s", "failed to get current status using stubs instead\n");
		goto use_stubs;
	}

	if (
		(CurStatus.st_mtim.tv_sec  != LastStatus->st_mtim.tv_sec) &&
		(CurStatus.st_mtim.tv_nsec != LastStatus->st_mtim.tv_nsec)
	   ) {
		if (*lhandmade) {
			if (dlclose(*lhandmade)) {
				fprintf(stderr, "%s", "dlclose error\n");
				exit(EXIT_FAILURE);
			}
		}
		*lhandmade = dlopen(HANDMADE_SO, RTLD_NOW);
		if (!*lhandmade) {
			fprintf(stderr, "%s", "fail to open shared object handmade hero " HANDMADE_SO "\n");
			fprintf(stderr, "%s\n", dlerror());
			fprintf(stderr, "%s", "using stubs\n");
			*lhandmade = NULL;
			goto use_stubs;
		}
		GameCode->GetController = dlsym(*lhandmade, "GetController");
		if (!GameCode->GetController) {
			dlclose(*lhandmade);
			fprintf(stderr, "%s", "dlsym error with GetController()\n");
			exit(EXIT_FAILURE);
		}
		GameCode->GameUpdate = dlsym(*lhandmade, "GameUpdate");
		if (!GameCode->GameUpdate) {
			dlclose(*lhandmade);
			fprintf(stderr, "%s", "dlsym error with GameUpdate()\n");
			exit(EXIT_FAILURE);
		}
		fprintf(stdout, "%s", "updated Handmade Hero lib successfully\n");
		LastStatus->st_mtim = CurStatus.st_mtim;
	}
	return;
use_stubs: {
		   GameCode->GetController = GetControllerStub;
		   GameCode->GameUpdate = GameUpdateStub;
		   return;
	   }
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
	errno = 0;
	int rc = 0;

	struct game_code GameCode = {};
	GameCode.GetController = GetControllerStub;
	GameCode.GameUpdate = GameUpdateStub;

	struct stat LastStatusHandmadeHeroLib = {};
	rc = stat(HANDMADE_SO, &LastStatusHandmadeHeroLib);
	if (-1 == rc) {
		fprintf(stderr, "%s", "failed to obtain the status of the shared object " HANDMADE_SO "\n");
		if (errno) {
			fprintf(stderr, "%s\n", strerror(errno));
		}
		exit(EXIT_FAILURE);
	}

	void *lhandmade = dlopen(HANDMADE_SO, RTLD_NOW);
	if (!lhandmade) {
		fprintf(stderr, "%s", "fail to open shared object handmade hero " HANDMADE_SO "\n");
		exit(EXIT_FAILURE);
	}
	GameCode.GetController = dlsym(lhandmade, "GetController");
	if (!GameCode.GetController) {
		fprintf(stderr, "%s", "failed to load GetController()\n");
		dlclose(lhandmade);
		exit(EXIT_FAILURE);
	}
	GameCode.GameUpdate = dlsym(lhandmade, "GameUpdate");
	if (!GameCode.GameUpdate) {
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
	int const width = HH_GAME_WINDOW_WIDTH;
	int const height = HH_GAME_WINDOW_HEIGHT;
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
	int const GameRateHz = ScreenRefreshRate / 2;
	float const ElapsedTimePerFrameNanoSec = 1.0e9 / ((float) GameRateHz);
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

	uint64 red_shift = 0;
	uint64 green_shift = 0;
	uint64 blue_shift = 0;

	uint64 const rgb_mask = 0xff;
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

	XSizeHints hints = {};
	hints.flags = (PMinSize | PMaxSize);
	hints.min_width = HH_GAME_WINDOW_WIDTH;
	hints.max_width = HH_GAME_WINDOW_WIDTH;
	hints.min_height = HH_GAME_WINDOW_HEIGHT;
	hints.max_height = HH_GAME_WINDOW_HEIGHT;
	XSetWMNormalHints(display, window, &hints);

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

	int32 Tilemap[HH_GAME_YTILECOUNT_TILEMAP][HH_GAME_XTILECOUNT_TILEMAP] = {
		{0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
		{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
		{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
		{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
	};

	uint64 const pitch = width * 4;
	struct game_state *GameState = Memory.PermanentStorage;
	Assert(INT_MAX >= red_shift);
	Assert(INT_MAX >= green_shift);
	Assert(INT_MAX >= blue_shift);
	GameState->RedShift = red_shift;
	GameState->GreenShift = green_shift;
	GameState->BlueShift = blue_shift;
	GameState->Player.XPos = 256;
	GameState->Player.YPos = 256;
	GameState->Player.Width = 64;
	GameState->Player.Height = 64;
	GameState->Player.XMin = GameState->Player.XPos;
	GameState->Player.XMax = GameState->Player.XPos + GameState->Player.Width;
	GameState->Player.YMin = GameState->Player.YPos;
	GameState->Player.YMax = GameState->Player.YPos + GameState->Player.Height;
	GameState->Player.Red = 1.0f;
	GameState->Player.Green = 1.0f;
	GameState->Player.Blue = 1.0f;
	GameState->Pitch = pitch;
	GameState->World.XTilemapCount = HH_GAME_XCOUNT_TILEMAP_WORLD;
	GameState->World.YTilemapCount = HH_GAME_YCOUNT_TILEMAP_WORLD;
	GameState->World.NumTilemaps = HH_GAME_NUM_TILEMAPS;
	GameState->World.TileSize = HH_GAME_TILESIZE;
	GameState->World.Width = HH_GAME_WIDTH_WORLD;
	GameState->World.Height = HH_GAME_HEIGHT_WORLD;
	for (int32 idx = 0; idx != HH_GAME_NUM_TILEMAPS; ++idx) {
		int const j = (idx / HH_GAME_XCOUNT_TILEMAP_WORLD);
		int const i = idx - (HH_GAME_YCOUNT_TILEMAP_WORLD * j);
		GameState->World.Tilemaps[idx].Id = idx;
		GameState->World.Tilemaps[idx].XId = i;
		GameState->World.Tilemaps[idx].YId = j;
		GameState->World.Tilemaps[idx].XPos = (
			(i * HH_GAME_WIDTH_TILEMAP)
		);
		GameState->World.Tilemaps[idx].YPos = (
			(j * HH_GAME_HEIGHT_TILEMAP)
		);
		GameState->World.Tilemaps[idx].Width = HH_GAME_WIDTH_TILEMAP;
		GameState->World.Tilemaps[idx].Height = HH_GAME_HEIGHT_TILEMAP;
		GameState->World.Tilemaps[idx].XCount = HH_GAME_XTILECOUNT_TILEMAP;
		GameState->World.Tilemaps[idx].YCount = HH_GAME_YTILECOUNT_TILEMAP;
		GameState->World.Tilemaps[idx].Length = (
			sizeof(*(GameState->World.Tilemaps[idx].Data)) *
			HH_GAME_XTILECOUNT_TILEMAP *
			HH_GAME_YTILECOUNT_TILEMAP
		);
		// NOTE: experimenting with memory alignment, here Size means that if we wanted
		// to stack tilemaps we can ensure memalignment this way between tilemaps for
		// performance. And we can also ensure that the address of the first tilemap
		// is also aligned.
		// TODO: consider adding a MACRO for alignment to a 32-byte boundary:
		//       Align(x) ((x) + 0x1f) & ~0x1f
		GameState->World.Tilemaps[idx].Size = (
			(GameState->World.Tilemaps[idx].Length + 0x1f) & ~0x1f
		);
		long const addr = (long) GameState;
		long const aligned = (
			((addr +
			((sizeof(*GameState))) +
			((idx * GameState->World.Tilemaps[idx].Size)) +
			0x1f) & ~0x1f)
		);
		GameState->World.Tilemaps[idx].Data = (
			(typeof(GameState->World.Tilemaps[idx].Data)) (aligned)
		);
		// TODO consider moving the tilemap initialization to a function that checks
		//      the tilemap dimensions
		memcpy(GameState->World.Tilemaps[idx].Data, Tilemap, sizeof(Tilemap));
	}

	uint8_t const red = 0x00;
	uint8_t const green = 0x00;
	uint8_t const blue = 0x00;
	uint64 const pixels = width * height;
	uint64 const framesz = pixels * 4;
	int *framebuffer = Memory.TransientStorage;
	for (uint64 i = 0; i != pixels; ++i) {
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
	uint64 frames = 0;
	struct timespec TimeStart = {};
	struct timespec TimeEnd = {};
	struct timespec TimeDelta = {};
	struct timespec TimeSum = {};
	struct timespec ClockFrameDuration = {};

	LinuxSetTimeSpec(&ClockFrameDuration, ElapsedTimePerFrameNanoSec);
	while (Running) {
		clock_gettime(CLOCK_MONOTONIC_RAW, &TimeStart);
		struct timespec ClockLastTime = {};
		struct timespec ClockTargetTime = {};
		clock_gettime(clockid, &ClockLastTime);
		LinuxSetDelayTime(&ClockTargetTime, &ClockLastTime, &ClockFrameDuration);

		struct game_controller_input *NewKeyboardController = GameCode.GetController(NewInput, 0);
		struct game_controller_input *OldKeyboardController = GameCode.GetController(OldInput, 0);
		memset(NewKeyboardController, 0, sizeof(*NewKeyboardController));
		for (
			uint32 ButtonIndex = 0;
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

		LinuxUpdateHandmadeLib(
			&LastStatusHandmadeHeroLib,
			&lhandmade,
			&GameCode
		);

		// NOTE: since we swapped the inputs ahead of time for timing purposes we pass the
		//       OldInput because it actually refers to the current input for this frame
		GameCode.GameUpdate(OldInput, &Memory, &Buffer);

		XPutImage(display, window, gc, image, 0, 0, 0, 0, width, height);
		LinuxDelay(clockid, &ClockTargetTime);
		clock_gettime(CLOCK_MONOTONIC_RAW, &TimeEnd);
		LinuxDiffTimeSpec(&TimeDelta, &TimeStart, &TimeEnd);
		LinuxCSumTimeSpec(&TimeSum, &TimeDelta);
		++frames;
	}

	float OverallGameRateHz = frames / (((float) TimeSum.tv_sec) + 1.0e-9 * ((float) TimeSum.tv_nsec));
	fprintf(stdout, "average fps: %.1f\n", OverallGameRateHz);

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
	if (lhandmade) {
		dlclose(lhandmade);
	}
	return 0;
}
