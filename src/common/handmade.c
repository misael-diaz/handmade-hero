#include "handmade.h"

void GameUpdate(
	struct game_memory *Memory,
	struct game_offscreen_buffer *Buffer
) {
	struct game_state *GameState = Memory->PermanentStorage;
	Assert((sizeof(*GameState) <= Memory->PermanentStorageSize));
	if (!Memory->Initialized) {
		GameState->GreenOffset = 0;
		GameState->BlueOffset = 0;

		// NOTE: this may be more appropriate to do in the platform layer
		Memory->Initialized = true;
	}
}
