#pragma once
#include "windows.h"


#define BUFSIZE 1024
#define CLASS_NAME TEXT("silly")
#define WINDOW_NAME "SillyTerm"


typedef struct thread_data {
  HANDLE hFile;
  wchar_t buffer[BUFSIZE];
  BOOL signal;
  DWORD sz;
} ThreadData;


DWORD WINAPI ReaderThread(LPVOID lparam);
DWORD WINAPI WriterThread(LPVOID lparam);
DWORD WINAPI KBDReaderThread(LPVOID lparam);

LRESULT CALLBACK WindowProcessMessages(HWND hwnd, UINT msg, WPARAM param, LPARAM lparam);

HRESULT sillyterm_init();
void sillyterm_run();
