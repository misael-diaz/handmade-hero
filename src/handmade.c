#include <stdint.h>
#include <stdbool.h>
#include <windows.h>

#define global static
#define local_persist static
#define internal static
#define BITMAP_DEPTH 32

typedef uint8_t uint8;
typedef uint32_t uint32;

// TODO this are global for now

global BOOL running;
global struct win32_offscreen_buffer {
	BITMAPINFO BitmapInfo;
	void *Bitmap;
	int BitmapPitch;
	int BitmapWidth;
	int BitmapHeight;
	int BitmapSize;
} BackBuffer;

struct win32_window_dimension {
	int Width;
	int Height;
};

internal struct win32_window_dimension Win32GetWindowDimension(HWND Window)
{
	RECT ClientRect = {};
	GetClientRect(Window, &ClientRect);
	struct win32_window_dimension WindowDimension = {};
	WindowDimension.Width = (ClientRect.right - ClientRect.left);
	WindowDimension.Height = (ClientRect.bottom - ClientRect.top);
	return WindowDimension;
}

// DIB Device Independent Bitmap
// Thanks to Chris Hecker of Spy party fame for his insightful comments (I'm keeping the
// thank you note as Casey did on the stream day 4 session).
internal void Win32ResizeDIBSection(
		struct win32_offscreen_buffer * const Buffer,
		int const Width,
		int const Height)
{
	Buffer->BitmapWidth = Width;
	Buffer->BitmapHeight = Height;
	// NOTE we want a bitmamp with a top-left origin so by convention biHeight < 0
	Buffer->BitmapInfo.bmiHeader.biSize = sizeof(Buffer->BitmapInfo.bmiHeader);
	Buffer->BitmapInfo.bmiHeader.biWidth = (Buffer->BitmapWidth);
	Buffer->BitmapInfo.bmiHeader.biHeight = -(Buffer->BitmapHeight);
	Buffer->BitmapInfo.bmiHeader.biPlanes = 1;
	Buffer->BitmapInfo.bmiHeader.biBitCount = BITMAP_DEPTH;
	Buffer->BitmapInfo.bmiHeader.biCompression = BI_RGB;
	if (Buffer->Bitmap) {
		VirtualFree(Buffer->Bitmap, 0, MEM_RELEASE);
		Buffer->Bitmap = NULL;
	}
	int const BitmapSize = (BITMAP_DEPTH / 8) * (Buffer->BitmapWidth * Buffer->BitmapHeight);
	Buffer->BitmapSize = BitmapSize;
	DWORD const AllocationType = MEM_COMMIT | MEM_RESERVE;
	Buffer->Bitmap = VirtualAlloc(0, BitmapSize, AllocationType, PAGE_READWRITE);
	uint8 *Row = Buffer->Bitmap;
	uint32 *Pixel = Buffer->Bitmap;
	int const Pitch = (BITMAP_DEPTH / 8) * Buffer->BitmapWidth;
	Buffer->BitmapPitch = Pitch;
	for(int y = 0; y != Buffer->BitmapHeight; ++y) {
		Pixel = (uint32*) Row;
		uint8 const R = 0xff; // Red
		uint8 const G = 0xff; // Green
		uint8 const B = 0xff; // Blue
		uint8 const P = 0x00; // Pad
		for(int x = 0; x != Buffer->BitmapWidth; ++x) {
			*(Pixel + x) = ((P << 24) + (R << 16) + (G << 8) + (B << 0));
		}
		Row += Pitch;
	}
}

internal void Win32CopyBufferToWindow(
		struct win32_offscreen_buffer Buffer,
		HDC DeviceContext,
		int const WindowWidth,
		int const WindowHeight,
		int const X,
		int const Y,
		int const Width,
		int const Height)
{
	StretchDIBits(
		DeviceContext,
/*
		X,
		Y,
		Width,
		Height,
		X,
		Y,
		Width,
		Height,
*/
		0,
		0,
		WindowWidth,
		WindowHeight,
		0,
		0,
		Buffer.BitmapWidth,
		Buffer.BitmapHeight,
		Buffer.Bitmap,
		&Buffer.BitmapInfo,
		DIB_RGB_COLORS,
		SRCCOPY
	);
}

LRESULT CALLBACK Win32MainWindowCallback(
		HWND Window,
		UINT Message,
		WPARAM WParam,
		LPARAM LParam)
{
	LRESULT Result = 0;
	switch(Message) {
		case WM_SIZE:
		{
			struct win32_window_dimension WindowDimension = Win32GetWindowDimension(Window);
			Win32ResizeDIBSection(&BackBuffer, WindowDimension.Width, WindowDimension.Height);
			OutputDebugString("WM_SIZE");
		} break;

		case WM_DESTROY:
		{
			// TODO treat this as an unexpected error and restore window?
			running = false;
		} break;

		case WM_CLOSE:
		{
			// TODO should display message to the player first?
			running = false;
		} break;

		case WM_ACTIVATEAPP:
		{
			OutputDebugString("WM_ACTIVEAPP");
		} break;

		case WM_PAINT:
		{
			OutputDebugString("WM_PAINT");
			PAINTSTRUCT Paint = {};
			HDC DeviceContext = BeginPaint(Window, &Paint);
			int const X = Paint.rcPaint.left;
			int const Y = Paint.rcPaint.top;
			int const Width = (Paint.rcPaint.right - Paint.rcPaint.left);
			int const Height = (Paint.rcPaint.bottom - Paint.rcPaint.top);
			struct win32_window_dimension WindowDimension = Win32GetWindowDimension(Window);
			Win32CopyBufferToWindow(
				BackBuffer,
				DeviceContext,
				WindowDimension.Width,
				WindowDimension.Height,
				X,
				Y,
				Width,
				Height);
			EndPaint(Window, &Paint);
		} break;

		case WM_SYSKEYDOWN:
		{
		} break;

		case WM_SYSKEYUP:
		{
		} break;

		case WM_KEYDOWN:
		{
		} break;

		case WM_KEYUP:
		{
			uint32 VKCode = WParam;
			bool const WasDown = ((LParam & (1 << 30)) != 0);
			if (VK_UP == VKCode) {
			} else if (VK_DOWN == VKCode) {
			} else if (VK_LEFT == VKCode) {
			} else if (VK_RIGHT == VKCode) {
			} else if (VK_SPACE == VKCode) {
			} else if (VK_ESCAPE == VKCode) {
				if (WasDown) {
					OutputDebugString("ESCAPE Key Previous State Bit Set");
				} else {
					OutputDebugString("ESCAPE Key Previous State Bit NOT Set");
				}
			}
		} break;

		default:
		{
			Result = DefWindowProc(Window, Message, WParam, LParam);
			//OutputDebugString("default");
		} break;
	}
}

int CALLBACK WinMain(
		HINSTANCE Instance,
		HINSTANCE PrevInstance,
		LPSTR CommandLine,
		int ShowCode)
{
	WNDCLASS WindowClass = {};
	WindowClass.style = CS_HREDRAW | CS_VREDRAW;
	WindowClass.lpfnWndProc = Win32MainWindowCallback;
	WindowClass.hInstance = Instance;
	WindowClass.lpszClassName = "HandMadeHeroWindowClass";
	if (RegisterClass(&WindowClass)) {
		HWND WindowHandle = CreateWindowEx(
			0,
			WindowClass.lpszClassName,
			"Handmade Hero",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			Instance,
			0
		);
		if (WindowHandle) {
			MSG Message = {};
			running = true;
			while (running) {
				BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
				if (0 < MessageResult) {
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				} else {
					break;
				}
			}
		}
	}
	return 0;
}
