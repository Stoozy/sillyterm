#include <windows.h>
#include "silly.h"

DWORD WINAPI WriterThread(LPVOID lparam){
  ThreadData * threadData = (ThreadData*)lparam;
  // TODO:
}

DWORD WINAPI ReaderThread(LPVOID lparam){
  ThreadData * threadData = (ThreadData*)lparam;
  DWORD availableBytes = 0;
  DWORD bytesRead = 0;
  wchar_t localBuf[BUFSIZE] = {0};

  for(;;){

    // OutputDebugStringA("[ReaderThread] running!");
    if(PeekNamedPipe(threadData->hFile, NULL, NULL, NULL, &availableBytes, NULL)){
      if(availableBytes != 0){
        if(availableBytes > BUFSIZE){
          // TODO: deal with this
          if(ReadFile(threadData->hFile, &localBuf, BUFSIZE, &bytesRead, NULL)){
            if(bytesRead != 0){
              memcpy(&threadData->buffer, &localBuf, bytesRead);
              threadData->signal = TRUE; // data is available, signal to whoever is listening
            }
          }
        }

        if(ReadFile(threadData->hFile, &localBuf, availableBytes, &bytesRead, NULL)){
          if(bytesRead != 0){
            memcpy(&threadData->buffer, &localBuf, bytesRead);
            threadData->signal = TRUE; // data is available, signal to whoever is listening
            OutputDebugStringA("Data read!\n");
          }
        }
      }
    }

  }

}