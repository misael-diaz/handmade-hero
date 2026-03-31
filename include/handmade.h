#ifndef HANDMADE_H
#define HANDMADE_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define internal static
#define HH_GAME_WINDOW_WIDTH 1600
#define HH_GAME_WINDOW_HEIGHT 900
#define ArrayCount(ary) (sizeof(ary) / sizeof(*ary))

#if HANDMADE_DEV
// portable way of crashing the application if assertion fails
#define Assert(expr)\
	if (!(expr)) {\
		fprintf(stderr, "assertion failed at %s:%d\n", __FILE__, __LINE__);\
		*((int *) 0) = 0;\
	}
#else
#define Assert(expr)
#endif

// analogous to Casey's approach, making platform functions static ensures that they cannot be called from
// the game layer and so these are effectively internal for other builds
#if HANDMADE_INTERNAL
#define DEBUG
#else
#define DEBUG static
#endif

#define KiloBytes(x) (1024LU * (x))
#define MegaBytes(x) (1024LU * 1024LU * (x))
#define GigaBytes(x) (1024LU * 1024LU * 1024LU * (x))

// IMPORTANT: this is not meant to be used in production code
struct debug_read_file_result {
	void *Data;
	size_t FileSize;
};

// Windows and GNU/Linux compatible game button state, for Linux we need to store whether the key was down
// but that does not incur in extra memory requirements because the struct is padded anyway for alignment
struct game_button_state {
	int HalfTransitionCount;
	bool EndedDown;
	bool WasDown;
	char _pad[2];
};
_Static_assert(8 == sizeof(struct game_button_state));

struct game_controller_input {
	float StartX;
	float StartY;

	float MinX;
	float MinY;

	float MaxX;
	float MaxY;

	float EndX;
	float EndY;

	union {
		struct game_button_state Buttons[6];
		struct {
			struct game_button_state Up;
			struct game_button_state Down;
			struct game_button_state Left;
			struct game_button_state Right;
			struct game_button_state LeftShoulder;
			struct game_button_state RightShoulder;
		};
	};

	bool IsAnalog;
};

// keyboard and four Xbox controllers
struct game_input {
	struct game_controller_input Controllers[5];
};

struct game_state {
	int GreenOffset;
	int BlueOffset;
};

// NOTE: we expect the permanent storage to be cleared to zero on initialization
struct game_memory {
	uint64_t PermanentStorageSize;
	uint64_t TransientStorageSize;
	void * PermanentStorage;
	void * TransientStorage;
	bool Initialized;
};

struct game_offscreen_buffer {
	void *Bitmap;
	int Pitch;
	int Width;
	int Height;
	int Size;
};

struct game_controller_input *GetController(
        struct game_input * const Input,
        int const ControllerIndex
);

void GameUpdate(
	struct game_input *Input,
	struct game_memory *Memory,
	struct game_offscreen_buffer *Buffer
);

struct game_code {
	struct game_controller_input *(*GetController)(
		struct game_input * const Input,
		int const ControllerIndex
	);

	void (*GameUpdate)(
		struct game_input *Input,
		struct game_memory *Memory,
		struct game_offscreen_buffer *Buffer
	);
};

#endif
