#include <sys/wait.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#define ACTIVE_READERS  0
#define WRITE_QUEUE     1
#define READ_QUEUE      2
#define ACTIVE_WRITER   3
#define BIN_WRITER      4

#define P -1
#define V  1

struct sembuf start_read_sbuf[] = {
  {READ_QUEUE, V, 0},
  {ACTIVE_WRITER, 0, 0},
  {WRITE_QUEUE, 0, 0},
  {ACTIVE_READERS, V, 0},
  {READ_QUEUE, P, 0},
};
struct sembuf stop_read_sbuf[] = {
  {ACTIVE_READERS, P, 0},
};
struct sembuf start_write_sbuf[] = {  
  {WRITE_QUEUE, V, 0},
  {ACTIVE_READERS, 0, 0},
  {BIN_WRITER, P, 0},
  {ACTIVE_WRITER, 0, 0},
  {ACTIVE_WRITER, V, 0},
  {WRITE_QUEUE, P, 0},
};
struct sembuf stop_write_sbuf[] = {
  {ACTIVE_WRITER, P, 0},
  {BIN_WRITER, V, 0},
};

int *number;
int shmid;
int semid;

int start_read()
{
    return semop(semid, start_read_sbuf, 5);
}
int stop_read()
{
    return semop(semid, stop_read_sbuf, 1);
}
int start_write()
{
    return semop(semid, start_write_sbuf, 6);
}
int stop_write()
{
    return semop(semid, stop_write_sbuf, 2);
}

int init_monitor()
{
    if (semctl(semid, ACTIVE_READERS, SETVAL, 0) == -1)
        return EXIT_FAILURE;
    if (semctl(semid, WRITE_QUEUE, SETVAL, 0) == -1)
        return EXIT_FAILURE;
    if (semctl(semid, READ_QUEUE, SETVAL, 0) == -1)
        return EXIT_FAILURE;
    if (semctl(semid, ACTIVE_WRITER, SETVAL, 0) == -1)
        return EXIT_FAILURE;
    if (semctl(semid, BIN_WRITER, SETVAL, 1) == -1)
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}


void log_exit(const char *msg)
{
  perror(msg);
  exit(EXIT_FAILURE);
}

void writer()
{
  srand(getpid());
  //for (int i = 0; i < 4; i++)
  while(1)
  {
    sleep(rand() % 5);
    int offset = rand() % 26;

    if (start_write() == -1)
    {
      perror("cant start_write");
      exit(EXIT_FAILURE);
    }
    (*number)++;
    printf("Writer incremented = \'%d\'\n", (*number));

    if (stop_write() == -1)
    {
      perror("cant stop_write");
      exit(EXIT_FAILURE);
    }
  }
  exit(EXIT_SUCCESS);
}

void reader()
{
  srand(getpid());
  //for (int i = 0; i < 4; i++)
  while(1)
  {
    sleep(rand() % 2);
    int offset = rand() % 26;

    if (start_read() == -1)
    {
      perror("cant start_read");
      exit(EXIT_FAILURE);
    }
    printf("Reader got value = \'%d\'\n", (*number));
    if (stop_read() == -1)
    {
      perror("cant stop_read");
      exit(EXIT_FAILURE);
    }
  }
  exit(EXIT_SUCCESS);
}

int main(void) 
{
  const int wr_count = 3;
  const int rd_count = 4;
  const int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

  key_t semkey = ftok("./key.txt", 1);
  if (semkey == -1)
    return EXIT_FAILURE;

  if ((semid = semget(semkey, 5, IPC_CREAT | perms)) == -1)
    return EXIT_FAILURE;

  key_t shmkey = ftok("./key.txt", 2);
  if (shmkey == -1)
    return EXIT_FAILURE;

  shmid = shmget(shmkey, sizeof(int), IPC_CREAT | perms);
  if (shmid == -1)
    return EXIT_FAILURE;

  number = shmat(shmid, 0, perms);
  if (number == (int *)-1)
    return EXIT_FAILURE;
  *number = 0;

  if (init_monitor())
    log_exit("cant init_monitor");

  pid_t chpid[wr_count + rd_count];
  for (int i = 0; i < wr_count; i++) 
  {
    if ((chpid[i] = fork()) == -1)
      log_exit("cant fork");
    else if (chpid[i] == 0)
      writer();
    else
    {}  
  }

  for (int i = wr_count; i < rd_count + wr_count; i++) 
  {
    if ((chpid[i] = fork()) == -1)
      log_exit("cant fork");
    else if (chpid[i] == 0)
      reader();
    else
    {}  
  }

  for (int i = 0; i < rd_count + wr_count; i++)
  {
    int status;
    if (waitpid(chpid[i], &status, WUNTRACED) == -1)
      log_exit("cant wait");

    if (WIFEXITED(status)) 
        printf("child with pid %d has finished, code: %d\n", chpid[i], WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        printf("child with pid %d has finished by unhandlable signal, signum: %d\n", chpid[i], WTERMSIG(status));
    else if (WIFSTOPPED(status))
        printf("child with pid %d has finished by signal, signum: %d\n", chpid[i], WSTOPSIG(status));
  }

  if (semctl(semid, 1, IPC_RMID, NULL) == -1)
    log_exit("Can't semctl\n");

  if (shmdt(number) == -1)
    log_exit("Can't shmdt\n");
  
  if (shmctl(shmid, IPC_RMID, NULL) == -1)
    log_exit("Can't shmctl\n");

  return EXIT_SUCCESS;
}