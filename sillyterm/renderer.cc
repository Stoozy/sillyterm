#include <Windows.h>
#include <d2d1.h>
#include <Dwrite.h>
#include "renderer.h"
#include "silly.h"



IDWriteFactory * pDWriteFactory_;
IDWriteTextFormat* pTextFormat_;
const wchar_t* wszText_;
UINT32 cTextLength_;

ID2D1Factory* pD2DFactory_;
ID2D1HwndRenderTarget* pRT_;
ID2D1SolidColorBrush* pWhiteBrush_;




#pragma comment(lib, "Dwrite")
#pragma comment(lib, "d2d1")

template <class T> void SafeRelease(T **ppT) {
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}


RECT rc;
void CreateDeviceResources(HWND hwnd_){
  GetClientRect(hwnd_, &rc);

  D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
  HRESULT hr = S_OK;

  if (!pRT_){
      // Create a Direct2D render target.
      hr = pD2DFactory_->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
                                                D2D1::HwndRenderTargetProperties(hwnd_, size),
                                                &pRT_);
      // Create a black brush.
      if (SUCCEEDED(hr))
          hr = pRT_->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),
                                           &pWhiteBrush_);
      else {
        OutputDebugStringA("CreateDeviceResources(): failed creating direct2d render target");
        DWORD x = GetLastError();
        exit(-1);
      }
  }

  return;
}

void DestroyDeviceResources() {
  SafeRelease(&pRT_);
  SafeRelease(&pWhiteBrush_);
}

#define dpiScaleX_ 1
#define dpiScaleY_ 1

void draw(){
  D2D1_RECT_F layoutRect = D2D1::RectF(
    static_cast<FLOAT>(rc.left) ,
    static_cast<FLOAT>(rc.top),
    static_cast<FLOAT>(rc.right - rc.left) ,
    static_cast<FLOAT>(rc.bottom - rc.top) );

  wszText_ = L"Hello World using  DirectWrite!";
  cTextLength_ = (UINT32) wcslen(wszText_);


  pRT_->DrawText(
    wszText_,        // The string to render.
    cTextLength_,    // The string's length.
    pTextFormat_,    // The text format.
    layoutRect,       // The region of the window where the text will be rendered.
    pWhiteBrush_);    // The brush used to draw the text.


  OutputDebugStringA("Drew Text\r\n");
}


void renderer_init(HWND hwnd){
  DWORD hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
                               &pD2DFactory_);
  if(SUCCEEDED(hr)){

    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
                             __uuidof(IDWriteFactory),
                             reinterpret_cast<IUnknown**>(&pDWriteFactory_));


    if (SUCCEEDED(hr)) {
        hr = pDWriteFactory_->CreateTextFormat(L"Consolas",                // Font family name.
                                               NULL,                       // Font collection (NULL sets it to use the system font collection).
                                               DWRITE_FONT_WEIGHT_REGULAR,
                                               DWRITE_FONT_STYLE_NORMAL,
                                               DWRITE_FONT_STRETCH_NORMAL,
                                               12.0f,
                                               L"en-us",
                                               &pTextFormat_);

        // Center align (horizontally) the text.
        if (SUCCEEDED(hr)) hr = pTextFormat_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        if (SUCCEEDED(hr)) hr = pTextFormat_->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    }
  }

  CreateDeviceResources(hwnd);

  pRT_->BeginDraw();
  pRT_->SetTransform(D2D1::IdentityMatrix());
  pRT_->Clear(D2D1::ColorF(D2D1::ColorF::Black));
  draw();
  pRT_->EndDraw();

  DestroyDeviceResources();
  return;
}
