#pragma once

#include "Windows.h"


void CreateDeviceResources(HWND hwnd);
void DestroyDeviceResources();



#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void renderer_init(HWND);
#undef EXTERNC
