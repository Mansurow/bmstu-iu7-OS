#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SIZE_BUF 26
#define PRODUCERS_AMOUNT 3
#define CONSUMERS_AMOUNT 3

#define SB 0
#define SE 1
#define SF 2

#define P -1
#define V  1

struct sembuf start_produce[2] = { {SE, P, 0}, {SB, P, 0} };
struct sembuf stop_produce[2] =  { {SB, V, 0}, {SF, V, 0} };
struct sembuf start_consume[2] = { {SF, P, 0}, {SB, P, 0} };
struct sembuf stop_consume[2] =  { {SB, V, 0}, {SE, V, 0} };

void producer(const int semid, const int shmid)
{
    char *addr = shmat(shmid, NULL, 0);
    if (addr == (char *) -1)
    {
        perror("Can't shmat.\n");
        exit(1);
    }

    char **prod_ptr = (char **) addr;
    char **cons_ptr = prod_ptr + sizeof(char);
    char *alpha_ptr = (char*) (cons_ptr + sizeof(char));

    srand(getpid());
    while(*alpha_ptr <= 'z')
    {
        int sem_op_p = semop(semid, start_produce, 2);
        if (sem_op_p == -1)
        {
            perror("Can't semop\n");
            exit(1);
        }

        **prod_ptr = *alpha_ptr;
        printf("Producer %d - %c\n", getpid(), **prod_ptr);

        (*prod_ptr)++;
        (*alpha_ptr)++;

        sleep(rand() % 2);

        int sem_op_v = semop(semid, stop_produce, 2);
        if (sem_op_v == -1)
        {
            perror("Can't semop\n");
            exit(1);
        }
    }

    if (shmdt((void *) prod_ptr) == -1)
    {
        perror("Can't shmdt.\n");
        exit(1);
    }

    exit(0);
}

void consumer(const int semid, const int shmid)
{   
    char *addr = shmat(shmid, NULL, 0);
    if (addr == (char *) -1)
    {
        perror("Can't shmat.\n");
        exit(1);
    }

    char **prod_ptr = (char **) addr;
    char **cons_ptr = prod_ptr + sizeof(char);
    char *alpha_ptr = (char*) (cons_ptr + sizeof(char));

    srand(getpid());
    while(**cons_ptr <= 'z')
    {
        int sem_op_p = semop(semid, start_consume, 2);
        if (sem_op_p == -1)
        {
            perror("Can't semop\n");
            exit(1);
        }
        
        printf("Consumer %d - %c\n", getpid(), **cons_ptr);
        (*cons_ptr)++;
        sleep(rand() % 3);

        int sem_op_v = semop(semid, stop_consume, 2);
        if (sem_op_v == -1)
        {
            perror("Can't semop\n");
            exit(1);
        }
    }

    if (shmdt((void *) addr) == -1)
    {
        perror("Can't shmdt.\n");
        exit(1);
    }

    exit(0);
}

int main()
{   
    int shmid, semid;
    // Значение IPC_PRIVATE указывает, что к разделяемой
    // памяти нельзя получить доступ другим процессам.
    // shmget - создает новый разделяемый сегмент.
    // S_IRUSR	Владелец может читать.
    // S_IWUSR	Владелец может писать.
    // S_IRGRP	Группа может читать.
    // S_IROTH	Остальные могут читать.
    int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    char** prod_ptr;
    char** cons_ptr;
    char* alpha_ptr;

    key_t shmkey = ftok("./key.txt", 1);
    if (shmkey == -1)
    {
        perror("Can't ftok for shm.\n");
        exit(1);
    }

    if ((shmid = shmget(shmkey, (SIZE_BUF + 3) * sizeof(char), IPC_CREAT | perms)) == -1)
    {
        perror("Can't shmget.\n");
        exit(1);
    }

    // Функция shmat() возвращает указатель на сегмент
    // shmaddr (второй аргумент) равно NULL,
    // то система выбирает подходящий (неиспользуемый)
    // адрес для подключения сегмента.
    char *buf = shmat(shmid, NULL, 0);
    if (buf == (char *) -1)
    {
        perror("Can't shmat.\n");
        exit(1);
    }

    prod_ptr = (char **) buf;
    cons_ptr = prod_ptr + sizeof(char);
    alpha_ptr = (char*) (cons_ptr + sizeof(char));

    *cons_ptr = alpha_ptr + sizeof(char);
    *prod_ptr = *cons_ptr;
    *alpha_ptr = 'a';

    key_t semkey = ftok("./key.txt", 1);
    if (semkey == -1)
    {
        perror("Can't ftok for sem.\n");
        exit(1);
    }

    if ((semid = semget(semkey, 3, IPC_CREAT | perms)) == -1)
    {
        perror("Can't semget.\n");
        exit(1);
    }

    // Создание семафоров (3 семафора)
    int c_sb = semctl(semid, SB, SETVAL, 1);
    int c_se = semctl(semid, SE, SETVAL, SIZE_BUF);
    int c_sf = semctl(semid, SF, SETVAL, 0);

    if (c_se == -1 || c_sf == -1 || c_sb == -1)
    {
        perror("Can't semctl.\n");
        exit(1);
    }

    pid_t chpid[CONSUMERS_AMOUNT + PRODUCERS_AMOUNT - 1];
    

    // Создание оставшихся производителей
    for (int i = 0; i < PRODUCERS_AMOUNT - 1; i++)
    {
        chpid[i] = fork();
        if (chpid[i] == -1)
        {
            perror("Can't fork producer.\n");
            exit(1);
        }

        if (chpid[i] == 0)
        {
            producer(semid, shmid);
        }
    }

    // Создание потребителей
    for (int i = PRODUCERS_AMOUNT - 1; i < CONSUMERS_AMOUNT + PRODUCERS_AMOUNT - 1; i++)
    {
        chpid[i] = fork();
        if (chpid[i] == -1)
        {
            perror("Can't fork consumer.\n");
            exit(1);
        }

        if (chpid[i] == 0)
        {
            consumer(semid, shmid);
        }
    }

    // Производитель (Parent)
    srand(getpid());
    while(*alpha_ptr <= 'z')
    {
        int sem_op_p = semop(semid, start_produce, 2);
        if (sem_op_p == -1)
        {
            perror("Can't semop\n");
            exit(1);
        }

        **prod_ptr = *alpha_ptr;
        printf("Producer %d - %c\n", getpid(), **prod_ptr);

        (*prod_ptr)++;
        (*alpha_ptr)++;

        sleep(rand() % 2);

        int sem_op_v = semop(semid, stop_produce, 2);
        if (sem_op_v == -1)
        {
            perror("Can't semop\n");
            exit(1);
        }
    }

    // Ожидание завершение всех children
    for (int i = 0; i < (CONSUMERS_AMOUNT + PRODUCERS_AMOUNT - 1); i++) 
    {
        int status;
        printf("Ожидание завершение - pid %d\n", chpid[i]);
        if (waitpid(chpid[i], &status, WUNTRACED) == -1)
        {
            perror("Can't waitpid.\n");
            exit(1);
        }

        if (WIFEXITED(status)) 
            printf("Child with pid %d has finished, code: %d\n", chpid[i], WEXITSTATUS(status));
        else if (WIFSIGNALED(status))
            printf("Child with pid %d has finished by unhandlable signal, signum: %d\n", chpid[i], WTERMSIG(status));
        else if (WIFSTOPPED(status))
            printf("Child with pid %d has finished by signal, signum: %d\n", chpid[i], WSTOPSIG(status));
    }

    if (shmdt((void *) prod_ptr) == -1)
    {
        perror("Can't shmdt.\n");
        exit(1);
    }

    // IPC_RMID используется для пометки сегмента как удаленного.
    if (semctl(semid, 1, IPC_RMID, NULL) == -1)
    {
        perror("Can't delete semafor.\n");
        exit(1);
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1) 
    {
        perror("Can't mark a segment as deleted.\n");
        exit(1);
    }
}