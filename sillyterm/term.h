#pragma once
#include "d2d1.h"
#include <Windows.h>



typedef enum char_decoration {
  // see: https://www.xfree86.org/current/ctlseqs.html
  NORMAL,
  BOLD,
  UNDERLINE,
  BLINK,
  NEGATIVE,
  HIDDEN,
  NORMAL2 = 22,
  NOT_UNDERLINED = 24,
  STEADY = 25,
  POSITIVE = 27,
  VISIBLE = 28,
  FG_COLOR_BLACK = 30,
  FG_COLOR_RED = 31,
  FG_COLOR_GREEN = 32,
  FG_COLOR_YELLOW = 33,
  FG_COLOR_BLUE = 34,
  FG_COLOR_MAGENTA  = 35,
  FG_COLOR_CYAN = 36,
  FG_COLOR_WHITE = 37,
  FG_COLOR_DEFAULT = 39,

  BG_COLOR_BLACK = 40,
  BG_COLOR_RED = 41,
  BG_COLOR_GREEN = 42,
  BG_COLOR_YELLOW = 43,
  BG_COLOR_BLUE = 44,
  BG_COLOR_MAGENTA = 45,
  BG_COLOR_CYAN = 46,
  BG_COLOR_WHITE = 47,
  BG_COLOR_DEFAULT = 49,
} CharacterDecoration;


typedef struct terminal_cell {
  D2D1_COLOR_F fgColor;
  D2D1_COLOR_F bgColor;
  wchar_t character;

  CharacterDecoration decoration;
} TCELL;

typedef struct term_line {
  TCELL * cells;
  BOOL dirty;
} TLINE;

typedef struct term_state {
  // Cursor Position
  UINT32 cx;
  UINT32 cy;

  FLOAT fontWidth;
  FLOAT fontHeight;

  UINT32 cols;
  UINT32 rows;

  BOOL showCursor;

  TLINE * lines;
 
} TSTATE;


#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC TSTATE ts;
EXTERNC VOID TerminalWrite(const wchar_t * str, UINT32 len);
EXTERNC HRESULT TerminalInit(HWND hwnd);
#undef EXTERNC

