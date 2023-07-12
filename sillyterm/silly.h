#pragma once
#include "windows.h"


#define FONT_SIZE 16.0f
#define FONT_WIDTH 12
#define FONT_HEIGHT 16
#define FONT_NAME "SF Mono"

#define BUFSIZE 1024
#define CLASS_NAME TEXT("silly")
#define WINDOW_NAME "SillyTerm"


typedef struct thread_data {
  HANDLE hFile;
  wchar_t buffer[BUFSIZE];
  DWORD sz;
  BOOL signal;
} ThreadData;


DWORD WINAPI ReaderThread(LPVOID lparam);
DWORD WINAPI WriterThread(LPVOID lparam);

LRESULT CALLBACK WindowProcessMessages(HWND hwnd, UINT msg, WPARAM param, LPARAM lparam);

// silly.c

void SillytermHandleKeyboard(HWND hwnd, WPARAM wParam, LPARAM lParam);
void SillytermHandlePaint(HWND hwnd, WPARAM wParam, LPARAM lParam);
void SillytermHandleResize(HWND hwnd, WPARAM wParam, LPARAM lParam);

HRESULT SillytermInit();
void SillytermRun();
