#ifndef HANDMADE_H
#define HANDMADE_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define internal static
#define HH_GAME_WINDOW_WIDTH 1600
#define HH_GAME_WINDOW_HEIGHT 900
#define HH_GAME_TILESIZE 80
#define HH_GAME_XTILECOUNT_TILEMAP 17
#define HH_GAME_YTILECOUNT_TILEMAP 9
#define HH_GAME_XCOUNT_TILEMAP_WORLD 2
#define HH_GAME_YCOUNT_TILEMAP_WORLD 2
#define HH_GAME_WIDTH_WORLD (\
	(HH_GAME_XCOUNT_TILEMAP_WORLD) *\
	(HH_GAME_XTILECOUNT_TILEMAP) *\
	(HH_GAME_TILESIZE)\
)
#define HH_GAME_HEIGHT_WORLD (\
	(HH_GAME_YCOUNT_TILEMAP_WORLD) *\
	(HH_GAME_YTILECOUNT_TILEMAP) *\
	(HH_GAME_TILESIZE)\
)
#define HH_GAME_NUM_TILEMAPS (\
	(HH_GAME_XCOUNT_TILEMAP_WORLD) *\
	(HH_GAME_YCOUNT_TILEMAP_WORLD)\
)
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

typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int32_t int32;
typedef int64_t int64;
typedef float  real32;
typedef double real64;
_Static_assert(sizeof(real32) == sizeof(int32), "surprising: sizeof(float) is not 32-bits\n");
_Static_assert(sizeof(real64) == sizeof(int64), "surprising: sizeof(double) is not 64-bits\n");

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

struct game_tilemap {
	int32 XCount;
	int32 YCount;
	int32 Length;
	int32 Size;
	int32 *Data;
	int32 _pad[2];
};

_Static_assert(32 == sizeof(struct game_tilemap));

struct game_world {
	int32 XTilemapCount;
	int32 YTilemapCount;
	int32 NumTilemaps;
	int32 TileSize;
	real32 Width;
	real32 Height;
	struct game_tilemap Tilemaps[HH_GAME_NUM_TILEMAPS];
};

// NOTE: experimental player data
struct game_player {
	real32 XPos;
	real32 YPos;
	real32 Width;
	real32 Height;
	real32 XMin;
	real32 XMax;
	real32 YMin;
	real32 YMax;
	real32 Red;
	real32 Green;
	real32 Blue;
};

struct game_state {
	uint64 Pitch;
	int32 GreenOffset;
	int32 BlueOffset;
	int32 RedShift;
	int32 GreenShift;
	int32 BlueShift;
	struct game_player Player;
	struct game_world World;
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
        uint32 const ControllerIndex
);

void GameUpdate(
	struct game_input *Input,
	struct game_memory *Memory,
	struct game_offscreen_buffer *Buffer
);

struct game_code {
	struct game_controller_input *(*GetController)(
		struct game_input * const Input,
		uint32 const ControllerIndex
	);

	void (*GameUpdate)(
		struct game_input *Input,
		struct game_memory *Memory,
		struct game_offscreen_buffer *Buffer
	);
};

#endif
