#include <Windows.h>
#include "silly.h"
#include "renderer.h"
#include "term.h"
#include <sys/timeb.h>


HANDLE inputReadSide, outputWriteSide; // - Close these after CreateProcess of child application with pseudoconsole objectr.
HANDLE outputReadSide, inputWriteSide; // - Hold onto these and use them for communication with the child through the pseudoconsole.

static BYTE keyboardState[256];


HRESULT PrepareStartupInformation(HPCON hpc, STARTUPINFOEX* psi) {
    // Prepare Startup Information structure
    STARTUPINFOEX si;
    ZeroMemory(&si, sizeof(si));
    si.StartupInfo.cb = sizeof(STARTUPINFOEX);

    // Discover the size required for the list
    size_t bytesRequired = 0;
    InitializeProcThreadAttributeList(NULL, 1, 0, &bytesRequired);

    // Allocate memory to represent the list
    si.lpAttributeList = (PPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(), 0, bytesRequired);
    if (!si.lpAttributeList)
    {
        return E_OUTOFMEMORY;
    }

    // Initialize the list memory location
    if (!InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &bytesRequired))
    {
        HeapFree(GetProcessHeap(), 0, si.lpAttributeList);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Set the pseudoconsole information into the list
    if (!UpdateProcThreadAttribute(si.lpAttributeList,
                                   0,
                                   PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
                                   hpc,
                                   sizeof(hpc),
                                   NULL,
                                   NULL))
    {
        HeapFree(GetProcessHeap(), 0, si.lpAttributeList);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    *psi = si;

    return S_OK;
}

HRESULT SetupPseudoConsole(COORD size) {
    HRESULT hr = S_OK;

    // Create communication channels
    if (!CreatePipe(&inputReadSide, &inputWriteSide, NULL, 0))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (!CreatePipe(&outputReadSide, &outputWriteSide, NULL, 0))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    HPCON hPC;
    hr = CreatePseudoConsole(size, inputReadSide, outputWriteSide, 0, &hPC);
    if(FAILED(hr)){
      OutputDebugStringA("Couldn't create ConPTY session.\n");
      exit(-1);
    }

    // PCWSTR childApplication = L"C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe";
    PCWSTR childApplication = L"C:\\Program Files\\Git\\bin\\bash.exe";
    // PCWSTR childApplication = L"C:\\windows\\system32\\cmd.exe";

    // Create mutable text string for CreateProcessW command line string.
    const size_t charsRequired = wcslen(childApplication) + 1; // +1 null terminator
    PWSTR cmdLineMutable = (PWSTR)HeapAlloc(GetProcessHeap(), 0, sizeof(wchar_t) * charsRequired);

    if (!cmdLineMutable)
    {
        return E_OUTOFMEMORY;
    }

    wcscpy_s(cmdLineMutable, charsRequired, childApplication);

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    STARTUPINFOEX siEx;
    ZeroMemory(&siEx, sizeof(siEx));

    PrepareStartupInformation(hPC, &siEx);

    // Call CreateProcess
    if (!CreateProcessW(NULL,
                        cmdLineMutable,
                        NULL,
                        NULL,
                        FALSE,
                        EXTENDED_STARTUPINFO_PRESENT,
                        NULL,
                        NULL,
                        &siEx.StartupInfo,
                        &pi))
    {
        HeapFree(GetProcessHeap(), 0, cmdLineMutable);
	exit(-1);

        return HRESULT_FROM_WIN32(GetLastError());
    }
    else {
      CloseHandle( pi.hProcess );
      CloseHandle( pi.hThread );

      CloseHandle(inputReadSide);
      CloseHandle(outputWriteSide);
    }

    return hr;
}


struct {
    BOOL shiftDown;
    BOOL ctrlDown;
    BOOL altDown;
    BOOL caps;
} kbdState =  {FALSE, FALSE, FALSE};



static void SillytermWriteToPTY(wchar_t * data, int len){
  size_t bytes = len * sizeof(wchar_t);
  size_t bytesWritten =0;

  WriteFile(inputWriteSide, data, bytes, &bytesWritten, 0);

  extern BOOL quit;
  if(bytesWritten != bytes){
    OutputDebugStringA("Couldn't write to console\n");
  }else{
    const char msg[256] = {0};
    sprintf_s(msg, 256, "SillytermWriteToPTY(): %d bytes were written\n\0", bytesWritten);
    OutputDebugStringA(msg);
  }

}

void SillytermHandleKeyboard(HWND hwnd, WPARAM wParam, LPARAM lParam){
    // TODO:
    // OutputDebugStringA("Got keyboard input!\n");

    WORD vkCode = LOWORD(wParam);                                 // virtual-key code
    WORD keyFlags = HIWORD(lParam);

    WORD scanCode = LOBYTE(keyFlags);                             // scan code
    BOOL isExtendedKey = (keyFlags & KF_EXTENDED) == KF_EXTENDED; // extended-key flag, 1 if scancode has 0xE0 prefix

    if (isExtendedKey)
        scanCode = MAKEWORD(scanCode, 0xE0);

    BOOL wasKeyDown = (keyFlags & KF_REPEAT) == KF_REPEAT;        // previous key-state flag, 1 on autorepeat
    WORD repeatCount = LOWORD(lParam);                            // repeat count, > 0 if several keydown messages was combined into one message

    BOOL isKeyReleased = (keyFlags & KF_UP) == KF_UP;             // transition-state flag, 1 on keyup

    // if we want to distinguish these keys:
    switch (vkCode) {
    case VK_CAPITAL: kbdState.caps = !kbdState.caps; break;
    case VK_SHIFT:   // converts to VK_LSHIFT or VK_RSHIFT
      if(isKeyReleased) kbdState.shiftDown = FALSE;
      else kbdState.shiftDown = TRUE;
      break;
    case VK_CONTROL: // converts to VK_LCONTROL or VK_RCONTROL
      if(isKeyReleased) kbdState.ctrlDown = FALSE;
      else kbdState.ctrlDown = TRUE;
      break;
    case VK_MENU:    // converts to VK_LMENU or VK_RMENU
      if(isKeyReleased) kbdState.altDown = FALSE;
      else kbdState.altDown = TRUE;
      break;
    case VK_BACK:{
      if(isKeyReleased){
	wchar_t DEL =  127;
	SillytermWriteToPTY(&DEL, 1);
      }


      break;
    }
    case VK_SPACE: {
      if(!isKeyReleased)
	SillytermWriteToPTY(L" ", 1);

      break;
    }
    case VK_RETURN:{
      OutputDebugStringA("Pressed ENTER.\n");
      if(!isKeyReleased)
	SillytermWriteToPTY(L"\r\n", 2);
      break;
    }
    default:{
      if(isKeyReleased)
	return;

      if(!GetKeyboardState(&keyboardState))
	OutputDebugStringA("Couldn't get keyboard state");

      char buf[16] = {0};
      int chars = ToAscii(vkCode, scanCode, &keyboardState, &buf, kbdState.altDown);
      char msg[256] = {0};
      sprintf_s( msg, 200, "SillytermHandleKbd(): Received %c \0", buf[0]);
      OutputDebugStringA(msg);
      OutputDebugStringA("\n");

      SillytermWriteToPTY(&buf, chars);
      break;
    }
    }
}

void SillytermHandlePaint(HWND hwnd, WPARAM wParam, LPARAM lParam){
    // TODO: call renderer code from here
}

void SillytermHandleResize(HWND hwnd, WPARAM wParam, LPARAM lParam){
    // TODO: resize pseudoconsole here
}


BOOL quit = FALSE;

void SillytermRun() {

  MSG msg;

#ifdef DEBUG_MODE
  struct timeb start, end;
  int frames = 0;
  FLOAT ms_elapsed = 0;
#endif

  wchar_t localBuf[BUFSIZE] = {0};
  wchar_t rbuf[BUFSIZE] = {0};
  DWORD availableBytes = 0;
  DWORD bytesRead = 0;

  HWND hwnd = 0;

  while(!quit){
#ifdef DEBUG_MODE
    ftime(&start);
#endif

    if(PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)){
      if(hwnd == 0){
	hwnd = msg.hwnd;
      }

      TranslateMessage(&msg);
      DispatchMessageA(&msg);
    }

    if(PeekNamedPipe( outputReadSide, NULL, NULL, NULL, &availableBytes, NULL)){
      if(availableBytes == 0) goto render;
      char msg[256] = {0};
      sprintf_s( msg, 200, "%d bytes available.\n\0", availableBytes);
      OutputDebugStringA(msg);


      if(availableBytes >= BUFSIZE){
	for(int i=0; i<=(availableBytes / BUFSIZE); i++){

	  if(ReadFile(outputReadSide, &localBuf, BUFSIZE, &bytesRead, NULL) && bytesRead != 0){
	    sprintf_s( msg, 200, "ReaderThread(): %d bytes read.\n\0", bytesRead);
	    OutputDebugStringA(msg);

	    int charsNeeded = MultiByteToWideChar(CP_UTF8, 0, localBuf, (int)strlen(localBuf), NULL, 0);
	    MultiByteToWideChar(CP_UTF8, 0, localBuf, (int)strlen(localBuf), &rbuf, charsNeeded);
	    TerminalWrite(rbuf, bytesRead);
	    bytesRead = 0;
	  }
	}
      }else{

	if(ReadFile(outputReadSide, &localBuf, availableBytes, &bytesRead, NULL) && bytesRead != 0){
	  sprintf_s( msg, 200, "ReaderThread(): %d bytes read.\n\0", bytesRead);
	  OutputDebugStringA(msg);

	  int charsNeeded = MultiByteToWideChar(CP_UTF8, 0, localBuf, (int)strlen(localBuf), NULL, 0);
	  MultiByteToWideChar(CP_UTF8, 0, localBuf, (int)strlen(localBuf), &rbuf, charsNeeded);
	  TerminalWrite(rbuf, bytesRead);
	  bytesRead = 0;
	}

      }
    }


  render:
    RendererDraw();

#ifdef DEBUG_MODE
    ftime(&end);

    ms_elapsed += (1000.0f * (end.time-start.time))
      + (end.millitm -start.millitm);

    if(ms_elapsed >= 1000.0f){
      // ~ a second has passed
      char buffer[256] = {0};
      sprintf_s(buffer, 256, "Sillyterm | %d FPS \n", frames);
      OutputDebugStringA(buffer);

      //SetWindowText(hwnd, buffer);
      frames = 0;
      ms_elapsed = 0;
    }

    frames++;
#endif

  }



  // TODO: cleanup
}


HRESULT SillytermInit(){

    COORD coord = { ts.cols, ts.rows };
    HRESULT hr = SetupPseudoConsole(coord);
    return S_OK;
}
