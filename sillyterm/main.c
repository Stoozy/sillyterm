#include <windows.h>
#include <stdio.h>

LRESULT CALLBACK WindowProcessMessages(HWND hwnd, UINT msg, WPARAM param, LPARAM lparam);

extern HRESULT sillyterm_init();
extern void sillyterm_run();
RECT drawRect = { 0, 0, 600, 800 };

#define BUFSIZE 1024
wchar_t display_buf[1024];

// TODO: create a main loop that:
// 1. Reads from client applications output (the 'shell' itself)
// 2. Display that output right away
// 3. Read from stdin and forward that to the client application

// Maybe in the future split this up into different threads

#define CLASS_NAME TEXT("silly")
#define WINDOW_NAME "SillyTerm"
int WINAPI WinMain(HINSTANCE currentInstance, HINSTANCE previousInstance, PSTR cmdline, INT cmdCount) {

	ZeroMemory(&display_buf, sizeof(wchar_t) * 1024);
	memcpy(display_buf, "Testing\0", strlen( "Testing\0"));
	WNDCLASSEX wc; 
	ZeroMemory(&wc, sizeof(wc));
		
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

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(window_handle, &ps);
	// All painting occurs here, between BeginPaint and EndPaint.
	FillRect(hdc, &ps.rcPaint, (HBRUSH)CreateSolidBrush(RGB(0, 0, 0)));


	if (display_buf[0]) {
		DrawTextA(hdc, &display_buf[0], -1, &drawRect, 0);
	}

	EndPaint(window_handle, &ps);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}

	return 0;
}

LRESULT CALLBACK WindowProcessMessages(HWND hwnd, UINT msg, WPARAM param, LPARAM lparam) {
	switch (msg) {
		case WM_CLOSE: {
			if (MessageBox(hwnd, L"Really quit?", L"My application", MB_OKCANCEL) == IDOK)
			    DestroyWindow(hwnd);
			exit(-1);
			break;
		}
		case WM_PAINT:
		{
		    PAINTSTRUCT ps;
		    HDC hdc = BeginPaint(hwnd, &ps);
		    // All painting occurs here, between BeginPaint and EndPaint.
			FillRect(hdc, &ps.rcPaint, (HBRUSH)CreateSolidBrush(RGB(0, 0, 0)));


			if (display_buf[0]) {
				DrawTextA(hdc, &display_buf[0], -1, &drawRect, 0);
			}

		    EndPaint(hwnd, &ps);
		}
		default: return DefWindowProcA(hwnd, msg, param, lparam);
	}

	return DefWindowProcA(hwnd, msg, param, lparam);
}
