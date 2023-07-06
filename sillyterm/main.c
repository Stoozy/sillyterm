#include <windows.h>
#include <stdio.h>

#include "silly.h"

RECT drawRect = { 0, 0, 800, 600 };

wchar_t kbdBuffer[BUFSIZE];


int WINAPI WinMain(HINSTANCE currentInstance, HINSTANCE previousInstance, PSTR cmdline, INT cmdCount) {

	OutputDebugStringA("Hello debugger");

	WNDCLASSEX wc; ZeroMemory(&wc, sizeof(wc));
		
	wc.hInstance = currentInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpfnWndProc = WindowProcessMessages;
	wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.cbSize = sizeof(wc);

	int reg_status = RegisterClassEx(&wc);
	if (!reg_status) {
		int err = GetLastError();
		exit(-1);
	}
	
	HWND window_handle = CreateWindowEx(0, CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, wc.hInstance, NULL);

	if (window_handle) {
		ShowWindow(window_handle, SW_SHOW);
		UpdateWindow(window_handle);
	}
	else {
		int err = GetLastError();
		exit(-1);
	}

	sillyterm_init();
	sillyterm_run(window_handle);

	return 0;
}


extern void sillyterm_handle_kbd(HWND hwnd, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowProcessMessages(HWND hwnd, UINT msg, WPARAM param, LPARAM lparam) {
	switch (msg) {
		case WM_CLOSE: {
			if (MessageBox(hwnd, L"Really quit?", L"My application", MB_OKCANCEL) == IDOK)
			    DestroyWindow(hwnd);
			exit(-1);
			break;
		}
		// Keyboard input, passthrough
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP: sillyterm_handle_kbd(hwnd, param, lparam); break;

		case WM_PAINT: {
		    PAINTSTRUCT ps;
		    HDC hdc = BeginPaint(hwnd, &ps);
		    // All painting occurs here, between BeginPaint and EndPaint.
			FillRect(hdc, &ps.rcPaint, (HBRUSH)CreateSolidBrush(RGB(0, 0, 0)));


		    EndPaint(hwnd, &ps);
		}
		default: return DefWindowProcA(hwnd, msg, param, lparam);
	}

	return DefWindowProcA(hwnd, msg, param, lparam);
}
