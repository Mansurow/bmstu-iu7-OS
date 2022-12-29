#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <windows.h>

#define READER_TIME_TO 600
#define WRITER_TIME_TO 600
#define DIFF 400

#define RNUM 3
#define WNUM 3

#define RRUNS 7
#define WRUNS 8

HANDLE _can_read;
HANDLE _can_write;
HANDLE _mutex;

LONG _waiting_readers = 0;
LONG _active_readers = 0;
LONG _waiting_writers = 0;

bool _active_writer = false;

int val = 0;

void startRead() {
  InterlockedIncrement(&_waiting_readers);
  WaitForSingleObject(_mutex, INFINITE);
  if (_waiting_writers || WaitForSingleObject(_can_write, 0) == WAIT_OBJECT_0) {
    WaitForSingleObject(_can_read, INFINITE);
  }

  SetEvent(_can_read);
  InterlockedIncrement(&_active_readers);
  InterlockedDecrement(&_waiting_readers);
  ReleaseMutex(_mutex);
}

void stopRead() {
  InterlockedDecrement(&_active_readers);
  if (_active_readers == 0) {
    ResetEvent(_can_read);
    SetEvent(_can_write);
  }
}

DWORD WINAPI readerRoutine(CONST LPVOID lp_params) {
  int inx = (int)lp_params;
  int sinterv;
  srand(time(NULL) + inx);

  for (int i = 0; i < RRUNS; i++) {
    sinterv = READER_TIME_TO + rand() % DIFF;
    Sleep(sinterv);
    startRead();
    printf("reader #%d -> %d (slept for %3d ms)\n", inx, val, sinterv);
    stopRead();
  }
  return 0;
}

void startWrite() {
  InterlockedIncrement(&_waiting_writers);
  if (_active_writer || WaitForSingleObject(_can_read, 0) == WAIT_OBJECT_0) {
    WaitForSingleObject(_can_write, INFINITE);
  }
  InterlockedDecrement(&_waiting_writers);
  _active_writer = true;
}

void stopWirite() {
  _active_writer = false;
  if (_waiting_readers) {
    SetEvent(_can_read);
  } else {
    SetEvent(_can_write);
  }
}

DWORD WINAPI writerRoutine(CONST LPVOID lp_params) {
  int inx = (int)lp_params;
  int sinterv;
  srand(time(NULL) + inx + RNUM);

  for (int i = 0; i < WRUNS; i++) {
    sinterv = WRITER_TIME_TO + rand() % DIFF;
    Sleep(sinterv);
    startWrite();
    val++;
    printf("writer #%d -> %d (slept for %4d ms)\n", inx, val, sinterv);
    stopWirite();
  }
  return 0;
}

int main() {
  setbuf(stdout, NULL);
  HANDLE r_threads[RNUM];
  HANDLE w_threads[WNUM];

  if ((_mutex = CreateMutex(NULL, FALSE, NULL)) == NULL) {
    perror("can't create mutex.");
    return -1;
  }
  if ((_can_read =
           CreateEvent(NULL, FALSE, FALSE, NULL) == NULL ||
           (_can_write = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)) {
    perror("can't create event.");
    return -1;
  }
  for (int i = 0; i < RNUM; i++) {
    if ((r_threads[i] = CreateThread(NULL, 0, readerRoutine, (LPVOID)i, 0,
                                     NULL)) == NULL) {
      perror("can't create reader thread.");
      return -1;
    }
  }

  for (int i = 0; i < WNUM; i++) {
    if ((w_threads[i] = CreateThread(NULL, 0, writerRoutine, (LPVOID)i, 0,
                                     NULL)) == NULL) {
      perror("can't create writer thread.");
      return -1;
    }
  }

  WaitForMultipleObjects(RNUM, r_threads, TRUE, INFINITE);
  WaitForMultipleObjects(WNUM, w_threads, TRUE, INFINITE);
  CloseHandle(_mutex);
  CloseHandle(_can_read);
  CloseHandle(_can_write);

  return 0;
}