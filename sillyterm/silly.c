#include <Windows.h>
#include "silly.h"


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

void sillyterm_handle_kbd(HWND hwnd, WPARAM wParam, LPARAM lParam){
    // TODO:
    OutputDebugStringA("Got keyboard input!\n");
}

void sillyterm_run(HWND window_handle) {
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);

        if(readerThreadData.signal){
            readerThreadData.signal = FALSE;
            OutputDebugStringA("Got signal at sillyterm_run() console output\n");
            // data available
            OutputDebugStringA(&readerThreadData.buffer);
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

    writerThreadData.hFile = outputReadSide;
    ZeroMemory(&writerThreadData.buffer, sizeof(writerThreadData.buffer));
    writerThreadData.signal =  FALSE;


    HANDLE readerThread =  CreateThread(NULL, 0, &ReaderThread, &readerThreadData, 0, NULL);
    HANDLE writerThread =  CreateThread(NULL, 0, &WriterThread, 0, 0, NULL);

}
