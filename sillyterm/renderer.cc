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
IDWriteTextLayout * pTextLayout_;

ID2D1SolidColorBrush * bgBrush;
ID2D1SolidColorBrush * fgBrush;



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


    D2D1_RENDER_TARGET_PROPERTIES rtp = D2D1::RenderTargetProperties();
    rtp.type = D2D1_RENDER_TARGET_TYPE_HARDWARE;
    D2D1_HWND_RENDER_TARGET_PROPERTIES hrtp = D2D1::HwndRenderTargetProperties(hwnd_, size);
    hrtp.presentOptions |= D2D1_PRESENT_OPTIONS_IMMEDIATELY;

    hr = pD2DFactory_->CreateHwndRenderTarget(rtp,
					      hrtp,
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


static BOOL prevFrameUpdated = FALSE;
void RendererDraw(){
  // OutputDebugStringA("RendererDraw(): called\n");
  RECT rc;
  GetClientRect(gHwnd, &rc);

  pRT->BeginDraw();
  pRT->SetTransform(D2D1::IdentityMatrix());

  if(prevFrameUpdated)
    pRT->Clear(D2D1::ColorF(D2D1::ColorF::Black));

  int update_count = 0;
  for(UINT32 i=0; i<ts.rows; i++){

    if(!ts.lines[i].dirty) continue;

    update_count++;
    prevFrameUpdated = TRUE;

    for(int j=0; j<ts.cols; j++){
      TCELL cell = ts.lines[i].cells[j];
      int left =  CELL_WIDTH * j;
      int top =  CELL_HEIGHT * i;
      D2D1_RECT_F cellRect = D2D1::RectF(left,
					 top,
					 left + CELL_WIDTH,
					 top + CELL_HEIGHT);


      HRESULT hr = pDWriteFactory_->CreateTextLayout(&cell.character,
						     1,
						     pTextFormat_,
						     CELL_WIDTH,
						     CELL_HEIGHT,
						     &pTextLayout_);

      D2D1_POINT_2F origin = D2D1::Point2F(static_cast<FLOAT>(left),
					   static_cast<FLOAT>(top));

      pRT->FillRectangle(cellRect, bgBrush);
      pRT->DrawTextLayout(origin, pTextLayout_, fgBrush);
      // pRT->DrawText(&termChar.character, 1, pTextFormat_, cell, pWhiteBrush_);

      SafeRelease(&pTextLayout_);
      // SafeRelease(&bgBrush);
      // SafeRelease(&fgBrush);
    }

    //ts.lines[i].dirty = FALSE;
    // OutputDebugStringA("RendererDraw(): Drew text!\n");
  }

  if(update_count == 0 )
    prevFrameUpdated = FALSE;

  // draw cursor

  int left =  CELL_WIDTH * ts.cx;
  int top  =  CELL_HEIGHT * ts.cy;

  D2D1_RECT_F cursorRect = D2D1::RectF(
    static_cast<FLOAT>( left ),
    static_cast<FLOAT>( top ),
    static_cast<FLOAT>( left + CELL_WIDTH),
      static_cast<FLOAT>( top + CELL_HEIGHT));
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

  hr = pRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black),
				  &bgBrush );
  if(FAILED(hr)){
    OutputDebugStringA("Couldn't create brush!\n'");
    exit(-1);
  }

  hr = pRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),
				  &fgBrush );
  if(FAILED(hr)){
    OutputDebugStringA("Couldn't create brush!\n'");
    exit(-1);
  }



  // DestroyDeviceResources();
  return;
}
