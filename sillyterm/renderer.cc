#include <Windows.h>
#include <d2d1.h>
#include <Dwrite.h>
#include "renderer.h"
#include "silly.h"
#include "term.h"

#pragma comment(lib, "Dwrite")
#pragma comment(lib, "d2d1")

static HWND gHwnd = 0;
volatile BOOL rendererActive = FALSE;

IDWriteFactory * pDWriteFactory_;
IDWriteTextFormat* pTextFormat_;

ID2D1Factory* pD2DFactory_;
ID2D1HwndRenderTarget* pRT;
ID2D1SolidColorBrush* pWhiteBrush_;

template <class T> void SafeRelease(T **ppT) {
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}


HRESULT CreateDeviceResources(HWND hwnd_){
  RECT rc;
  GetClientRect(hwnd_, &rc);

  D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
  HRESULT hr = S_OK;

  if (!pRT){
      // Create a Direct2D render target.
      hr = pD2DFactory_->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
                                                D2D1::HwndRenderTargetProperties(hwnd_, size),
                                                &pRT);
      // Create a black brush.
      if (SUCCEEDED(hr))
          hr = pRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),
                                           &pWhiteBrush_);
      else {
        OutputDebugStringA("CreateDeviceResources(): failed creating direct2d render target");
        DWORD x = GetLastError();
        hr = E_FAIL;
      }
  }

  return hr;
}

void DestroyDeviceResources() {
  SafeRelease(&pRT);
  SafeRelease(&pWhiteBrush_);
}


void RendererDraw(){
  OutputDebugStringA("RendererDraw(): called\n");
  RECT rc;
  GetClientRect(gHwnd, &rc);

  pRT->BeginDraw();
  pRT->SetTransform(D2D1::IdentityMatrix());
  pRT->Clear(D2D1::ColorF(D2D1::ColorF::Black));

  for(UINT32 i=0; i<terminalState.lines; i++){
    UINT32 len = terminalState.cols;


    ID2D1SolidColorBrush * bgBrush;
    ID2D1SolidColorBrush * fgBrush;

    for(int j=0; j<len; j++){
        TerminalCharacter termChar = terminalState.screen[i][j];
        int left =  12 * j;
        int top =  18 * i;
        D2D1_RECT_F cell = D2D1::RectF(left, top, left + 12, top + 18);


        // TODO: only do this when the next color is different
        HRESULT hr = pRT->CreateSolidColorBrush(termChar.bgColor, &bgBrush );
        if(FAILED(hr)){
            OutputDebugStringA("Couldn't create brush!\n'");
            exit(-1);
        }
        hr = pRT->CreateSolidColorBrush(termChar.fgColor, &fgBrush );
        if(FAILED(hr)){
            OutputDebugStringA("Couldn't create brush!\n'");
            exit(-1);
        }


        pRT->FillRectangle(cell, bgBrush);
        pRT->DrawText(&termChar.character, 1, pTextFormat_, cell, fgBrush);

        SafeRelease(&bgBrush);
        SafeRelease(&fgBrush);
    }

    // OutputDebugStringA("RendererDraw(): Drew text!\n");
  }

  // draw cursor

  int left =  12 * terminalState.cx;
  int top =  18 * terminalState.cy;

  D2D1_RECT_F cursorRect = D2D1::RectF(
    static_cast<FLOAT>( left ),
    static_cast<FLOAT>( top ),
    static_cast<FLOAT>( left + 12),
    static_cast<FLOAT>( top + 18));

  pRT->FillRectangle(cursorRect, pWhiteBrush_);


  pRT->EndDraw();
}


void RendererInit(HWND hwnd){
  gHwnd = hwnd;
  DWORD hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
                               &pD2DFactory_);
  if(SUCCEEDED(hr)){

    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
                             __uuidof(IDWriteFactory),
                             reinterpret_cast<IUnknown**>(&pDWriteFactory_));


    if (SUCCEEDED(hr)) {
      hr = pDWriteFactory_->CreateTextFormat(TEXT(FONT_NAME),
					     NULL,
					     DWRITE_FONT_WEIGHT_REGULAR,
					     DWRITE_FONT_STYLE_NORMAL,
					     DWRITE_FONT_STRETCH_NORMAL,
					     FONT_SIZE,
					     L"en-us",
					     &pTextFormat_);

        // Center align (horizontally) the text.
        if (SUCCEEDED(hr)) hr = pTextFormat_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        if (SUCCEEDED(hr)) hr = pTextFormat_->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

    }
  }

  hr = CreateDeviceResources(hwnd);

  if(SUCCEEDED(hr))
    rendererActive = TRUE;

  // DestroyDeviceResources();
  return;
}
