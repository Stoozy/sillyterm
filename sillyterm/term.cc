#include "silly.h"
#include "term.h"
#include "vt100.h"

volatile TerminalState terminalState;

static void TerminalScreenInit(unsigned cols, unsigned lines){
  terminalState.screen = (TerminalCharacter**)HeapAlloc(GetProcessHeap(), 0, sizeof(TerminalCharacter *) * lines);
  for(UINT32 i=0; i<lines; i++){
      size_t sz = sizeof(TerminalCharacter )  * (cols + 1);
      terminalState.screen[i] = (TerminalCharacter*)HeapAlloc(GetProcessHeap(), 0, sz);
      ZeroMemory(terminalState.screen[i], sz);

      terminalState.screen[i][cols].character = 0; // null terminate line
  }
}


VOID TerminalWrite(const wchar_t * buf, UINT32 len){
    for(UINT32 i=0; i<len; i++){
      vt_handle_code((UINT32)buf[i]);
    }
}

HRESULT TerminalInit(HWND hwnd){

  terminalState.cx =  terminalState.cy = 0;

  terminalState.fontWidth = FONT_HEIGHT;
  terminalState.fontHeight = FONT_WIDTH;

  RECT rc;
  GetClientRect(hwnd, &rc);

  // int winWidth = rc.right-rc.left;
  // int winHeight = rc.bottom-rc.top;

  int winWidth = 800;
  int winHeight = 600;

  terminalState.cols = winWidth/terminalState.fontWidth;
  terminalState.lines = winHeight/terminalState.fontHeight;

  TerminalScreenInit(terminalState.cols, terminalState.lines);

  return S_OK;
}
