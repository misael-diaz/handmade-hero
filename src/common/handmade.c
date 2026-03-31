#include "handmade.h"

struct game_controller_input *GetController(
	struct game_input * const Input,
	int const ControllerIndex
) {
	Assert(0 <= ControllerIndex);
	size_t const Index = ControllerIndex;
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
}
