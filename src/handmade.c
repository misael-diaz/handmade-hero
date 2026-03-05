#include <stdint.h>
#include <windows.h>

#define global static
#define local_persist static
#define internal static
#define BITMAP_DEPTH 32

typedef uint8_t uint8;
typedef uint32_t uint32;

// TODO this is global for now

global BOOL running;
global BITMAPINFO BitmapInfo;
global void *Bitmap;
global int BitmapWidth;
global int BitmapHeight;

// DIB Device Independent Bitmap
// Thanks to Chris Hecker of Spy party fame for his insightful comments (I'm keeping the
// thank you note as Casey did on the stream day 4 session).
internal void Win32ResizeDIBSection(int const Width, int const Height)
{
	BitmapWidth = Width;
	BitmapHeight = Height;
	// NOTE we want a bitmamp with a top-left origin so by convention biHeight < 0
	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
	BitmapInfo.bmiHeader.biWidth = BitmapWidth;
	BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = BITMAP_DEPTH;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;
	if (Bitmap) {
		VirtualFree(Bitmap, 0, MEM_RELEASE);
		Bitmap = NULL;
	}
	int const BitmapSize = (BITMAP_DEPTH / 8) * (BitmapWidth * BitmapHeight);
	DWORD const AllocationType = MEM_COMMIT | MEM_RESERVE;
	Bitmap = VirtualAlloc(0, BitmapSize, AllocationType, PAGE_READWRITE);
	uint8 *Row = Bitmap;
	uint32 *Pixel = Bitmap;
	size_t const Pitch = (BITMAP_DEPTH / 8) * BitmapWidth;
	for(int y = 0; y != BitmapHeight; ++y) {
		Pixel = (uint32*) Row;
		uint8 const R = 0xff; // Red
		uint8 const G = 0xff; // Green
		uint8 const B = 0xff; // Blue
		uint8 const P = 0x00; // Pad
		for(int x = 0; x != BitmapWidth; ++x) {
			*(Pixel + x) = ((P << 24) + (R << 16) + (G << 8) + (B << 0));
		}
		Row += Pitch;
	}
}

internal void Win32UpdateWindow(
		HDC DeviceContext,
		RECT const * const WindowRect,
		int const X,
		int const Y,
		int const Width,
		int const Height)
{
	int const WindowWidth = WindowRect->right - WindowRect->left;
	int const WindowHeight = WindowRect->bottom - WindowRect->top;
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
		BitmapWidth,
		BitmapHeight,
		0,
		0,
		WindowWidth,
		WindowHeight,
		Bitmap,
		&BitmapInfo,
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
			RECT ClientRect = {};
			GetClientRect(Window, &ClientRect);
			int const Width = (
				ClientRect.right - ClientRect.left
			);
			int const Height = (
				ClientRect.bottom - ClientRect.top
			);
			Win32ResizeDIBSection(Width, Height);
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
			RECT WindowRect = {};
			GetWindowRect(Window, &WindowRect);
			Win32UpdateWindow(
				DeviceContext,
				&WindowRect,
				X,
				Y,
				Width,
				Height);
			EndPaint(Window, &Paint);
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
	WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
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
