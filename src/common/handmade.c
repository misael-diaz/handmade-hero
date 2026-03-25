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
	Assert((sizeof(*GameState) <= Memory->PermanentStorageSize));
	if (!Memory->Initialized) {
		GameState->GreenOffset = 0;
		GameState->BlueOffset = 0;

		// NOTE: this may be more appropriate to do in the platform layer
		Memory->Initialized = true;
	}

	if (Keyboard->Down.EndedDown) {
		GameState->GreenOffset++;
	}
}
