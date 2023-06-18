#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

pthread_mutex_t mutex;

struct args_struct 
{ 
    int fd; 
};

void *read_buf(void *args)
{
    struct args_struct *cur_args = (struct args_struct *) args;
    int fd = cur_args->fd;
    int flag = 1;
    pthread_mutex_lock(&mutex);
    while (flag == 1)
    {
        char c;
        if ((flag = read(fd, &c, 1)) == 1)
            printf("%c", c);
    }
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main()
{ 
    int fd1 = open("alphabet.txt", O_RDONLY);
    struct args_struct args1 = { .fd = fd1 };

    int fd2 = open("alphabet.txt", O_RDONLY);
    struct args_struct args2 = { .fd = fd2 };

    pthread_t td1, td2;
    pthread_create(&td1, NULL, read_buf, &args1);
    pthread_create(&td2, NULL, read_buf, &args2);
    pthread_join(td1, NULL);
    pthread_join(td2, NULL);
    puts("");
    return 0;
}