#include "handmade.h"

internal inline int32 TruncateReal32ToInt32(
	real32 value
) {
	return ((int32) (value));
}

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

internal inline int32 Real32ColorToRGB(
	real32 const Real32Color
) {
	return TruncateReal32ToInt32(255.0f * Real32Color);
}

internal void DrawRectangle(
	struct game_memory * const Memory,
	real32 xmin,
	real32 xmax,
	real32 ymin,
	real32 ymax,
	real32 Red,
	real32 Green,
	real32 Blue
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
	int32 const R = Real32ColorToRGB(Red);
	int32 const G = Real32ColorToRGB(Green);
	int32 const B = Real32ColorToRGB(Blue);
	int32 const RedShift = GameState->RedShift;
	int32 const GreenShift = GameState->GreenShift;
	int32 const BlueShift = GameState->BlueShift;
	framebuffer += (YMin * pitch);
	for (uint32 y = YMin; y != YMax; ++y) {
		int32 *rect = (int32*) framebuffer;
		for (uint32 x = XMin; x != XMax; ++x) {
			rect[x] = ((R << RedShift) | (G << GreenShift) | (B << BlueShift));
		}
		framebuffer += pitch;
	}
}

internal void DrawTilemap(
	struct game_memory * const Memory
) {
	struct game_state * const GameState = Memory->PermanentStorage;
	struct game_tilemap const * const Tilemap = &GameState->World.Tilemaps[0];
	int32 Pixels = 80;
	real32 Red = 0.0f;
	real32 Green = 0.0f;
	real32 Blue = 0.0f;
	for (int32 y = 0; y != Tilemap->YCount; ++y) {
		real32 const ymin = Pixels * y;
		real32 const ymax = Pixels * (y + 1);
		for (int32 x = 0; x != Tilemap->XCount; ++x) {
			int32 const Code = *(Tilemap->Data + (Tilemap->XCount * y) + x);
			if (0 == Code) {
				Red = 0.5f;
				Green = 0.5f;
				Blue = 0.5f;
			} else {
				Red = 0.0f;
				Green = 0.0f;
				Blue = 0.0f;
			}
			real32 const xmin = Pixels * x;
			real32 const xmax = Pixels * (x + 1);
			DrawRectangle(
					Memory,
					xmin,
					xmax,
					ymin,
					ymax,
					Red,
					Green,
					Blue
				     );
		}
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

internal real32 HandleButtonInput(
		struct game_button_state const * const Button
) {
	int32 ButtonPressCount = 0;
	if (Button->EndedDown) {
		ButtonPressCount = (Button->HalfTransitionCount >> 1);
	} else {
		if (Button->HalfTransitionCount & 1) {
			ButtonPressCount = 1 + (Button->HalfTransitionCount >> 1);
		} else {
			ButtonPressCount = (Button->HalfTransitionCount >> 1);
		}
	}
	return ButtonPressCount;
}

internal void DrawPlayer(
	struct game_input * const Input,
	struct game_memory * const Memory
) {
	struct game_controller_input *Keyboard = GetController(Input, 0);
	struct game_state *GameState = Memory->PermanentStorage;

	GameState->Player.XPos -= 10 * HandleButtonInput(&Keyboard->Left);
	GameState->Player.XPos += 10 * HandleButtonInput(&Keyboard->Right);
	GameState->Player.YPos -= 10 * HandleButtonInput(&Keyboard->Up);
	GameState->Player.YPos += 10 * HandleButtonInput(&Keyboard->Down);

	GameState->Player.XPos = Clamp(
		GameState->Player.XPos,
		0,
		HH_GAME_WINDOW_WIDTH - GameState->Player.Width - 2
	);

	GameState->Player.YPos = Clamp(
		GameState->Player.YPos,
		0,
		HH_GAME_WINDOW_HEIGHT - GameState->Player.Height - 2
	);

	GameState->Player.XMin = GameState->Player.XPos;
	GameState->Player.XMax = GameState->Player.XPos + GameState->Player.Width;
	GameState->Player.YMin = GameState->Player.YPos;
	GameState->Player.YMax = GameState->Player.YPos + GameState->Player.Height;

	DrawRectangle(
		Memory,
		GameState->Player.XMin,
		GameState->Player.XMax,
		GameState->Player.YMin,
		GameState->Player.YMax,
		GameState->Player.Red,
		GameState->Player.Green,
		GameState->Player.Blue
	);
}

void GameUpdate(
	struct game_input *Input,
	struct game_memory *Memory,
	struct game_offscreen_buffer *Buffer
) {
	struct game_state *GameState = Memory->PermanentStorage;
	int *framebuffer = Memory->TransientStorage;
	Assert((sizeof(*GameState) <= Memory->PermanentStorageSize));
	if (!Memory->Initialized) {
		GameState->GreenOffset = 0;
		GameState->BlueOffset = 0;

		// NOTE: this may be more appropriate to do in the platform layer
		Memory->Initialized = true;
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
	real32 const Red = 1.0f;
	real32 const Green = 1.0f;
	real32 const Blue = 1.0f;
	DrawTilemap(Memory);
	DrawPlayer(Input, Memory);
}
