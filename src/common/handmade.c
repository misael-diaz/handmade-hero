#include "handmade.h"

internal inline uint32 RoundReal32ToUInt32(
	real32 value
) {
	Assert(0.0f <= value);
	value += 0.5f;
	return ((uint32) value);
}

internal inline real32 Clamp(
	real32 const xval,
	real32 const xmin,
	real32 const xmax
) {
	if (xmin > xval) {
		return xmin;
	} else if (xmax < xval) {
		return xmax;
	} else {
		return xval;
	}
}

// TODO: extend to set the rectangle color
internal void DrawRectangle(
	struct game_memory * const Memory,
	real32 xmin,
	real32 xmax,
	real32 ymin,
	real32 ymax
) {
	struct game_state *GameState = Memory->PermanentStorage;
	char unsigned *framebuffer = Memory->TransientStorage;
	Assert(GameState);
	Assert(framebuffer);

	xmin = Clamp(xmin, 0.0f, (real32) HH_GAME_WINDOW_WIDTH);
	xmax = Clamp(xmax, 0.0f, (real32) HH_GAME_WINDOW_WIDTH);
	ymin = Clamp(ymin, 0.0f, (real32) HH_GAME_WINDOW_HEIGHT);
	ymax = Clamp(ymax, 0.0f, (real32) HH_GAME_WINDOW_HEIGHT);

	if (xmin > xmax) {
		real32 const temp = xmax;
		xmax = xmin;
		xmin = temp;
	}

	if (ymin > ymax) {
		real32 const temp = ymax;
		ymax = ymin;
		ymin = temp;
	}

	Assert(xmin < xmax);
	Assert(ymin < ymax);
	Assert(0.0f <= xmin);
	Assert(0.0f <= xmax);
	Assert(0.0f <= ymin);
	Assert(0.0f <= ymax);
	Assert(((real32) HH_GAME_WINDOW_WIDTH ) >= xmin);
	Assert(((real32) HH_GAME_WINDOW_WIDTH ) >= xmax);
	Assert(((real32) HH_GAME_WINDOW_HEIGHT) >= ymin);
	Assert(((real32) HH_GAME_WINDOW_HEIGHT) >= ymax);

	uint32 const XMin = RoundReal32ToUInt32(xmin);
	uint32 const XMax = RoundReal32ToUInt32(xmax);
	uint32 const YMin = RoundReal32ToUInt32(ymin);
	uint32 const YMax = RoundReal32ToUInt32(ymax);
	uint64 const pitch = GameState->Pitch;
	framebuffer += (YMin * pitch);
	for (uint32 y = YMin; y != YMax; ++y) {
		int *rect = (int*) framebuffer;
		for (uint32 x = XMin; x != XMax; ++x) {
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

	real32 const xmin = 256;
	real32 const ymin = 256;
	real32 const RectangleWidth = 64;
	real32 const RectangleHeight = 64;
	real32 const xmax = xmin + RectangleWidth;
	real32 const ymax = ymin + RectangleHeight;
	DrawRectangle(Memory, xmin, xmax, ymin, ymax);
}
