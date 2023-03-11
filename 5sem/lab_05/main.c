#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <stdbool.h>

HANDLE canRead;
HANDLE canWrite;
HANDLE mutex;

LONG activeReaders = 0;
LONG activeWriter = FALSE;
LONG waitingWriters = 0;
LONG waitingReaders = 0;

int number = 0;

void startRead()
{
  InterlockedIncrement(&waitingReaders);
  if (waitingWriters || WaitForSingleObject(canWrite, 0) == WAIT_OBJECT_0)
    WaitForSingleObject(canRead, INFINITE);
  WaitForSingleObject(mutex, INFINITE);
  SetEvent(canRead);
  InterlockedDecrement(&waitingReaders);
  InterlockedIncrement(&activeReaders);
  ReleaseMutex(mutex);
}

void stopRead()
{
  InterlockedDecrement(&activeReaders);
  if (activeReaders == 0) 
    SetEvent(canWrite);
}

void startWrite()
{
  InterlockedIncrement(&waitingWriters);
  if (activeWriter || WaitForSingleObject(canRead, 0) == WAIT_OBJECT_0)
    WaitForSingleObject(canWrite, INFINITE);
  WaitForSingleObject(mutex, INFINITE);
  InterlockedDecrement(&waitingWriters);
  InterlockedExchange(&activeWriter, TRUE);
  ReleaseMutex(mutex);
}

void stopWrite()
{
  ResetEvent(canWrite);
  InterlockedExchange(&activeWriter, FALSE);
  if (waitingReaders > 0)
    SetEvent(canRead);
  else
    SetEvent(canWrite);
}

void LogExit(const char *msg)
{
  perror(msg);
  ExitProcess(EXIT_SUCCESS);
}

DWORD Reader(PVOID param)
{
  srand(GetCurrentThreadId());
  for (int i = 0; i < 6; i++)
  {
    Sleep(rand() % 200 + 100);
    startRead();
    printf("Reader got value = %d\n", number);
    stopRead();
  }
  return EXIT_SUCCESS;
}

DWORD Writer(PVOID param) 
{
  srand(GetCurrentThreadId());
  for (int i = 0; i < 6; i++)
  {
    Sleep(rand() % 600);
    startWrite();
    number++;
    printf("Writer incremented = %d\n", number);
    stopWrite();
  }
  return EXIT_SUCCESS;
}

int main(void) 
{
  setbuf(stdout, NULL);

  const int writerAmount = 3;
  const int readerAmount = 4;
  DWORD thid[writerAmount + readerAmount];
  HANDLE pthread[writerAmount + readerAmount];

  if ((canRead = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
    LogExit("cant createEvent");
  if ((canWrite = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
    LogExit("cant createEvent");
  if ((mutex = CreateMutex(NULL, 0, NULL)) == NULL)
    LogExit("cant createMutex");

  for (int i = 0; i < writerAmount; i++)
  {
    pthread[i] = CreateThread(NULL, 0, Writer, NULL, 0, &thid[i]);
    if (pthread[i] == NULL)
      LogExit("cant createThread");
  }
  for (int i = writerAmount; i < writerAmount + readerAmount; i++)
  {
    pthread[i] = CreateThread(NULL, 0, Reader, NULL, 0, &thid[i]);
    if (pthread[i] == NULL)
      LogExit("cant createThread");
  }

  for (int i = 0; i < writerAmount + readerAmount; i++) 
  {
    DWORD dw = WaitForSingleObject(pthread[i], INFINITE);
    switch (dw) 
    {
      case WAIT_OBJECT_0:
        printf("thread %d finished\n", thid[i]);
        break;
      case WAIT_TIMEOUT:
        printf("waitThread timeout %d\n", dw);
        break;
      case WAIT_FAILED:
        printf("waitThread failed %d\n", dw);
        break;
      default:
        printf("unknown %d\n", dw);
        break;
    }
  }

  CloseHandle(canRead);
  CloseHandle(canWrite);
  CloseHandle(mutex);
  return EXIT_SUCCESS;
}