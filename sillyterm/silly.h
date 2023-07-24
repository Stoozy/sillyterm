#pragma once
#include "windows.h"

extern UINT32 CELL_WIDTH;
extern UINT32 CELL_HEIGHT;

#define DEBUG_MODE

#define FONT_SIZE 14.0f
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


LRESULT CALLBACK WindowProcessMessages(HWND hwnd, UINT msg, WPARAM param, LPARAM lparam);

// silly.c

void SillytermHandleKeyboard(HWND hwnd, WPARAM wParam, LPARAM lParam);
void SillytermHandlePaint(HWND hwnd, WPARAM wParam, LPARAM lParam);
void SillytermHandleResize(HWND hwnd, WPARAM wParam, LPARAM lParam);

HRESULT SillytermInit();
void SillytermRun();
