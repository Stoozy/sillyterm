#include "term.h"

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
        if(terminalState.cx == terminalState.cols || buf[i] == '\n'){
            terminalState.cx = 0;
            terminalState.cy++;
        }else{
            terminalState.screen[terminalState.cy][terminalState.cx].bgColor = D2D1::ColorF(D2D1::ColorF::Blue);
            terminalState.screen[terminalState.cy][terminalState.cx].fgColor = D2D1::ColorF(D2D1::ColorF::White);
            terminalState.screen[terminalState.cy][terminalState.cx++].character = buf[i];
        }
    }
}

HRESULT TerminalInit(HWND hwnd){

  terminalState.cx =  terminalState.cy = 0;

  terminalState.fontWidth = 13;
  terminalState.fontHeight = 16;

  RECT rc;
  GetClientRect(hwnd, &rc);

  // int winWidth = rc.right-rc.left;
  // int winHeight = rc.bottom-rc.top;

  int winWidth = 800;
  int winHeight = 600;

  terminalState.cols = winWidth/terminalState.fontWidth;
  terminalState.lines = winHeight/terminalState.fontHeight;

  TerminalScreenInit(terminalState.cols, terminalState.lines);

  // const char msg[256];
  // sprintf_s( msg, 200, "TerminalInit(): %d columns \n\0", terminalState.cols);
  // OutputDebugStringA(msg);

  // sprintf_s( msg, 200, "TerminalInit(): %d lines \n\0", terminalState.lines);
  // OutputDebugStringA(msg);

  return S_OK;
}
