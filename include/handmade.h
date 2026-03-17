#ifndef HANDMADE_H
#define HANDMADE_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#if HANDMADE_DEV
// portable way of crashing the application if assertion fails
#define Assert(expr) \
	if (!(expr)) {\
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

// TODO rename these since they are just placeholders for the actual data members and maybe the typing also
struct game_button_state {
	int Pressed;
	int Released;
};

struct game_controller_input {
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
};

struct game_input {
	struct game_controller_input Controllers[4];
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

void GameUpdate(
	struct game_memory *Memory,
	struct game_offscreen_buffer *Buffer
);

#endif
