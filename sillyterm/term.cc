#include "silly.h"
#include "term.h"
#include "vt100.h"

#include <d2d1.h>
#include <Dwrite.h>

volatile TerminalState terminalState;
UINT32 CELL_WIDTH = 0;
UINT32 CELL_HEIGHT = 0;


extern ID2D1Factory * pD2DFactory_;
extern IDWriteFactory * pDWriteFactory_;



static void TerminalScreenInit(unsigned cols, unsigned lines){
  terminalState.screen = (TerminalCharacter**)HeapAlloc(GetProcessHeap(), 0, sizeof(TerminalCharacter *) * lines);

  for(UINT32 i=0; i<lines; i++){
    size_t sz = sizeof(TerminalCharacter)  * (cols + 1);
    terminalState.screen[i] = (TerminalCharacter*)HeapAlloc(GetProcessHeap(), 0, sz);
    if(!terminalState.screen[i]){
      MessageBox(NULL, L"Couldn't allocate screen", 0, MB_OK );
      exit(-1);
    }

    for(UINT32 j=0; j<cols; j++){
      terminalState.screen[i][j].fgColor = D2D1::ColorF(D2D1::ColorF::White);
      terminalState.screen[i][j].bgColor = D2D1::ColorF(D2D1::ColorF::Blue);
    }

    ZeroMemory(terminalState.screen[i], sz);
    terminalState.screen[i][cols].character = 0; // null terminate line
  }

  terminalState.cols = cols;
  terminalState.lines = lines;
}


static void TerminalIncLine(){
  if(terminalState.cy+1 < terminalState.lines){
    terminalState.cy++;
  }else{
    // scroll buffer

    // instead of copying the memory line by line, just
    // rearrange the pointers to point to the next line;
    // delete the first line and create a new allocation for the last line
    HeapFree(GetProcessHeap(), 0, terminalState.screen[0]);

    int i=0;
    for(;i<terminalState.lines-1; i++)
      terminalState.screen[i] =  terminalState.screen[i+1];

    int cols = terminalState.cols;
    size_t sz = sizeof(TerminalCharacter)  * (cols + 1);
    terminalState.screen[i] = (TerminalCharacter*)HeapAlloc(GetProcessHeap(), 0, sz);
    terminalState.screen[i][cols].character = 0; // null terminate line
    ZeroMemory(terminalState.screen[i], sz);

  }

  terminalState.cx = 0;
}



VOID TerminalWrite(const wchar_t * buf, UINT32 len){
    for(UINT32 i=0; i<len; i++){
      if(buf[i] == '\n') TerminalIncLine();
      else vt_handle_code((UINT32)buf[i]);

      // OutputDebugStringA((LPCSTR)&buf[i]);
      // OutputDebugStringA("\n");
    }
}


static void GetCellDimensions(const wchar_t wc){

  extern IDWriteTextFormat* pTextFormat_;
  extern IDWriteFactory * pDWriteFactory_;
  IDWriteTextLayout * tl;
  HRESULT hr = pDWriteFactory_->CreateTextLayout(&wc,
						 1,
						 pTextFormat_,
						 INT_MAX,
						 INT_MAX,
						 &tl);
  DWRITE_TEXT_METRICS  metrics;
  tl->GetMetrics(&metrics);

  CELL_WIDTH = metrics.widthIncludingTrailingWhitespace;
  CELL_HEIGHT = metrics.height;

  return;
}


HRESULT TerminalInit(HWND hwnd){

  vt_init();


  terminalState.cx = terminalState.cy = 0;


  RECT rc;
  GetClientRect(hwnd, &rc);

  GetCellDimensions('W');

  int winWidth = rc.right-rc.left;
  int winHeight = rc.bottom-rc.top;

  terminalState.fontWidth = CELL_WIDTH;
  terminalState.fontHeight = CELL_HEIGHT;

  int cols = winWidth/terminalState.fontWidth;
  int lines = winHeight/terminalState.fontHeight;

  TerminalScreenInit(cols, lines);

  return S_OK;
}
