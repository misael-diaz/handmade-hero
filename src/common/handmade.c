#include "handmade.h"

internal int Clamp(
	int const xval,
	int const xmin,
	int const xmax
) {
	if (xmin > xval) {
		return xmin;
	} else if (xmax < xval) {
		return xmax;
	} else {
		return xval;
	}
}

internal void DrawRectangle(
	struct game_memory * const Memory,
	int xmin,
	int xmax,
	int ymin,
	int ymax
) {
	struct game_state *GameState = Memory->PermanentStorage;
	char unsigned *framebuffer = Memory->TransientStorage;
	Assert(GameState);
	Assert(framebuffer);

	xmin = Clamp(xmin, 0, HH_GAME_WINDOW_WIDTH);
	xmax = Clamp(xmax, 0, HH_GAME_WINDOW_WIDTH);
	ymin = Clamp(ymin, 0, HH_GAME_WINDOW_HEIGHT);
	ymax = Clamp(ymax, 0, HH_GAME_WINDOW_HEIGHT);

	if (xmin > xmax) {
		int const temp = xmax;
		xmax = xmin;
		xmin = temp;
	}

	if (ymin > ymax) {
		int const temp = ymax;
		ymax = ymin;
		ymin = temp;
	}

	Assert(xmin < xmax);
	Assert(ymin < ymax);
	Assert(0 <= xmin);
	Assert(0 <= xmax);
	Assert(0 <= ymin);
	Assert(0 <= ymax);
	Assert(HH_GAME_WINDOW_WIDTH  >= xmin);
	Assert(HH_GAME_WINDOW_WIDTH  >= xmax);
	Assert(HH_GAME_WINDOW_HEIGHT >= ymin);
	Assert(HH_GAME_WINDOW_HEIGHT >= ymax);

	size_t const XMin = xmin;
	size_t const XMax = xmax;
	size_t const YMin = ymin;
	size_t const YMax = ymax;
	size_t const pitch = GameState->Pitch;
	framebuffer += (YMin * pitch);
	for (size_t y = YMin; y != YMax; ++y) {
		int *rect = (int*) framebuffer;
		for (size_t x = XMin; x != XMax; ++x) {
			rect[x] = 0;
		}
		framebuffer += pitch;
	}
}

struct game_controller_input *GetController(
	struct game_input * const Input,
	uint32 const ControllerIndex
) {
	uint32 const Index = ControllerIndex;
	Assert(Index < ArrayCount(Input->Controllers));
	return &Input->Controllers[Index];
}

void GameUpdate(
	struct game_input *Input,
	struct game_memory *Memory,
	struct game_offscreen_buffer *Buffer
) {
	struct game_controller_input *Keyboard = GetController(Input, 0);
	struct game_state *GameState = Memory->PermanentStorage;
	int *framebuffer = Memory->TransientStorage;
	Assert((sizeof(*GameState) <= Memory->PermanentStorageSize));
	if (!Memory->Initialized) {
		GameState->GreenOffset = 0;
		GameState->BlueOffset = 0;

		// NOTE: this may be more appropriate to do in the platform layer
		Memory->Initialized = true;
	}

	if (Keyboard->Up.EndedDown) {
		GameState->GreenOffset += 16;
	}

	if (Keyboard->Down.EndedDown) {
		GameState->GreenOffset -= 16;
	}

	if (256 <= GameState->GreenOffset) {
		GameState->GreenOffset = 255;
	}

	if (0 >= GameState->GreenOffset) {
		GameState->GreenOffset = 0;
	}

	// NOTE: as long the game window is hardcoded this should work, we are experimenting so it's okay
	long unsigned pixels = (HH_GAME_WINDOW_WIDTH * HH_GAME_WINDOW_HEIGHT);
	for(long unsigned i = 0; i != pixels; ++i) {
		int const red = 0;
		int const green = GameState->GreenOffset;
		int const blue = 0;
		int const red_shift = GameState->RedShift;
		int const green_shift = GameState->GreenShift;
		int const blue_shift = GameState->BlueShift;
		framebuffer[i] = (
			(red   << red_shift) +
			(green << green_shift) +
			(blue  << blue_shift)
		);
	}

	int const xmin = 256;
	int const ymin = 256;
	int const RectangleWidth = 64;
	int const RectangleHeight = 64;
	int const xmax = xmin + RectangleWidth;
	int const ymax = ymin + RectangleHeight;
	DrawRectangle(Memory, xmin, xmax, ymin, ymax);
}
