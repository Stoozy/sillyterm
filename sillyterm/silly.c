#include <Windows.h>
#include "silly.h"
#include "renderer.h"

extern wchar_t kbdBuffer[BUFSIZE];

ThreadData readerThreadData;
ThreadData writerThreadData;

HRESULT PrepareStartupInformation(HPCON hpc, STARTUPINFOEX* psi)
{
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


HANDLE inputReadSide, outputWriteSide; // - Close these after CreateProcess of child application with pseudoconsole object.
HANDLE outputReadSide, inputWriteSide; // - Hold onto these and use them for communication with the child through the pseudoconsole.

HRESULT SetupPseudoConsole(COORD size)
{
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

    PCWSTR childApplication = L"C:\\windows\\system32\\cmd.exe";

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
        CloseHandle(inputReadSide);
        CloseHandle(outputWriteSide);
    }

    return hr;
}

wchar_t buf[BUFSIZE];

struct {
    BOOL shiftDown;
    BOOL ctrlDown;
    BOOL altDown;
    BOOL caps;
} kbdState =  {FALSE, FALSE, FALSE};


void sillyterm_handle_kbd(HWND hwnd, WPARAM wParam, LPARAM lParam){
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
    switch (vkCode)
    {
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
    case VK_RETURN:{
        const char * cmd = "echo Hello, World!\r\n";
        strcpy_s(&writerThreadData.buffer, strlen(cmd)+1, cmd);
        writerThreadData.sz = strlen(cmd);
        writerThreadData.signal =  TRUE;
        break;
    }
    default:{
        // ascii chars
        if(vkCode <= 0x5A && vkCode >= 0x41){
            if(isKeyReleased)
                return;
            if(!kbdState.shiftDown && !kbdState.caps) vkCode += 0x20;

            const char msg[256];
            sprintf_s( msg, 200, "sillyterm_handle_kbd(): %c was pressed \n\0", vkCode);
            OutputDebugStringA(msg);
        }else{
            const char msg[256];
            sprintf_s( msg, 200, "sillyterm_handle_kbd(): Unhandled vkCode : 0x%x\n\0", vkCode);
            OutputDebugStringA(msg);
        }

        break;
    }
    }

}

void sillyterm_handle_paint(HWND hwnd, WPARAM wParam, LPARAM lParam){
    // TODO: call renderer code from here
}

void sillyterm_handle_resize(HWND hwnd, WPARAM wParam, LPARAM lParam){
    // TODO: resize pseudoconsole here
}

void sillyterm_run() {
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);

        if(readerThreadData.signal){
            readerThreadData.signal = FALSE;
            OutputDebugStringA("Got signal at sillyterm_run() console output\n");
            // data available
            OutputDebugStringA((LPCSTR) & readerThreadData.buffer);
            OutputDebugStringA("\n");
            ZeroMemory(&readerThreadData.buffer, sizeof(readerThreadData.buffer));
        }

    }
}


HRESULT sillyterm_init() {
    COORD coord = { 80, 32 };
    SetupPseudoConsole(coord);

    readerThreadData.hFile = outputReadSide;
    ZeroMemory(&readerThreadData.buffer, sizeof(readerThreadData.buffer));
    readerThreadData.signal = FALSE;

    writerThreadData.hFile = inputWriteSide;
    ZeroMemory(&writerThreadData.buffer, sizeof(writerThreadData.buffer));

    const char * cmd = "echo Hello, World!\r\n";
    strcpy_s(&writerThreadData.buffer, strlen(cmd)+1, cmd);
    writerThreadData.sz = strlen(cmd);
    writerThreadData.signal =  TRUE;

    HANDLE readerThread =  CreateThread(NULL, 0, &ReaderThread, &readerThreadData, 0, NULL);
    HANDLE writerThread =  CreateThread(NULL, 0, &WriterThread, &writerThreadData, 0, NULL);

}
