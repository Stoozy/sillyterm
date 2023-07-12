#pragma once
#include "d2d1.h"
#include <Windows.h>

typedef enum decoration {
  NONE,
  BOLD,
  ITALIC,
  UNDERLINE
} CharacterDecoration;


typedef struct term_char {
    D2D1_COLOR_F fgColor;
    D2D1_COLOR_F bgColor;
    CharacterDecoration decoration;
    wchar_t character;

} TerminalCharacter;

typedef struct term_state {
  // Cursor Position
  UINT32 cx;
  UINT32 cy;

  FLOAT fontWidth;
  FLOAT fontHeight;

  UINT32 cols;
  UINT32 lines;

  BOOL showCursor;

  // Text buffer lines x cols
  TerminalCharacter ** screen;

} TerminalState;


#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC volatile TerminalState terminalState;
EXTERNC VOID TerminalWrite(const wchar_t * str, UINT32 len);
EXTERNC HRESULT TerminalInit(HWND hwnd);
#undef EXTERNC

