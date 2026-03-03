#include <windows.h>

LRESULT CALLBACK MainWindowCallback(
		HWND Window,
		UINT Message,
		WPARAM WParam,
		LPARAM LParam)
{
	LRESULT Result = 0;
	switch(Message) {
		case WM_SIZE:
		{
			OutputDebugString("WM_SIZE");
		} break;

		case WM_DESTROY:
		{
			OutputDebugString("WM_DESTROY");
		} break;

		case WM_CLOSE:
		{
			OutputDebugString("WM_CLOSE");
		} break;

		case WM_ACTIVATEAPP:
		{
			OutputDebugString("WM_ACTIVEAPP");
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
	WindowClass.lpfnWndProc = MainWindowCallback;
	WindowClass.hInstance = Instance;
	WindowClass.lpszClassName = "HandMadeHeroWindowClass";
	if (RegisterClass(&WindowClass)) {
		HWND WindowHandle = CreateWindowEx(
			0,
			WindowClass.lpszClassName,
			"Handmade Hero",
			WS_EX_OVERLAPPEDWINDOW | WS_VISIBLE,
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
			for (;;) {
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
