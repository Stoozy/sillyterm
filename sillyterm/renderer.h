#pragma once
#include "Windows.h"
#include "term.h"


#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC ID2D1SolidColorBrush * bgBrush;
EXTERNC ID2D1SolidColorBrush * fgBrush;


EXTERNC volatile BOOL rendererActive;
EXTERNC volatile BOOL rendererActive;
EXTERNC HRESULT CreateDeviceResources(HWND);
EXTERNC void DestroyDeviceResources();

EXTERNC void RendererDraw();
EXTERNC void RendererInit(HWND);

#undef EXTERNC
