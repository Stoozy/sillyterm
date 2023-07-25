#include "silly.h"
#include "term.h"
#include "vt100.h"

#include <d2d1.h>
#include <Dwrite.h>
#include <assert.h>

TSTATE ts;
UINT32 CELL_WIDTH = 0;
UINT32 CELL_HEIGHT = 0;


extern ID2D1Factory * pD2DFactory_;
extern IDWriteFactory * pDWriteFactory_;



static void TerminalScreenInit(unsigned cols, unsigned rows){
  ts.cols = cols;
  ts.rows = rows;


  ts.lines = (TLINE*)HeapAlloc(GetProcessHeap(), 0, sizeof(TLINE) * rows);
  assert(ts.lines != NULL);

  for(int i=0; i<rows; i++){
    ts.lines[i].cells = (TCELL*)HeapAlloc(GetProcessHeap(), 0, sizeof(TCELL) * cols);
    ZeroMemory(ts.lines[i].cells,  sizeof(TCELL) * cols);
    assert(ts.lines[i].cells != NULL);

    TCELL * cells = ts.lines[i].cells;
    for(int j=0; j<cols; j++){
      cells[j].fgColor = D2D1::ColorF(D2D1::ColorF::White);
      cells[j].bgColor = D2D1::ColorF(D2D1::ColorF::Blue);
    }

    ts.lines[i].dirty = TRUE;
  }
}


static void TerminalIncLine(){
  if(ts.cy+1 < ts.rows){
    ts.cy++;
  }else{
    // scroll buffer

    // instead of copying the memory line by line, just
    // rearrange the pointers to point to the next line;
    // delete the first line and create a new allocation for the last line
    HeapFree(GetProcessHeap(), 0, ts.lines[0].cells);

    int i=0;
    for(;i<ts.rows-1; i++){
      ts.lines[i].cells =  ts.lines[i+1].cells;
      ts.lines[i].dirty = TRUE;
    }

    size_t sz = sizeof(TCELL)  * ts.cols;
    ts.lines[i].cells = (TCELL*)HeapAlloc(GetProcessHeap(), 0, sz);
    ZeroMemory(ts.lines[i].cells, sz);

  }

  ts.cx = 0;
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


  ts.cx = ts.cy = 0;


  RECT rc;
  GetClientRect(hwnd, &rc);

  GetCellDimensions('W');

  int winWidth = rc.right-rc.left;
  int winHeight = rc.bottom-rc.top;

  ts.fontWidth = CELL_WIDTH;
  ts.fontHeight = CELL_HEIGHT;

  int cols = winWidth/ts.fontWidth;
  int rows = winHeight/ts.fontHeight;

  TerminalScreenInit(cols, rows);

  return S_OK;
}
