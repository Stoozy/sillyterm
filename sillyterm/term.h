#pragma once
#include "d2d1.h"
#include <Windows.h>

typedef struct term_char {
    D2D1_COLOR_F fgColor;
    D2D1_COLOR_F bgColor;
    wchar_t character;
} TerminalCharacter;

typedef struct term_state {
  // Cursor Position
  UINT32 cx;
  UINT32 cy;

  // font stuff
  FLOAT fontWidth;
  FLOAT fontHeight;

  // dimensions
  UINT32 cols;
  UINT32 lines;

  // Text buffer lines x cols
  
  TerminalCharacter ** screen;

} TerminalState;


#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC VOID TerminalWrite(const wchar_t * str, UINT32 len);
EXTERNC volatile TerminalState terminalState;
EXTERNC HRESULT TerminalInit(HWND hwnd);
#undef EXTERNC

