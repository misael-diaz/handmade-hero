#ifndef HANDMADE_H
#define HANDMADE_H

#include <stdint.h>
#include <stdbool.h>

#define KiloBytes(x) (1024 * (x))
#define MegaBytes(x) (1024 * 1024 * (x))
#define GigaBytes(x) (1024 * 1024 * 1024 * (x))

struct game_input {
};

struct game_state {
	int GreenOffset;
	int BlueOffset;
};

struct game_memory {
	uint64_t PermanentStorageSize;
	void * PermanentStorage;
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
