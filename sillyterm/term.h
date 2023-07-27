#pragma once
#include "d2d1.h"
#include <Windows.h>

// normalized values 0-1
typedef struct rgb {
  FLOAT r;
  FLOAT g;
  FLOAT b;
} RGB;


typedef struct terminal_cell {
  struct rgb fgColor;
  struct rgb bgColor;

  BOOL bold;
  BOOL italic;
  BOOL hidden;
  BOOL negative;
  BOOL strikethrough;
  BOOL dim;

  BOOL showCursor;
  wchar_t character;
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

EXTERNC const struct rgb colors[256];
EXTERNC TSTATE ts;
EXTERNC VOID TerminalWrite(const wchar_t * str, UINT32 len);
EXTERNC HRESULT TerminalInit(HWND hwnd);
#undef EXTERNC
