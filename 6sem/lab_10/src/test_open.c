#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

void file_info()
{
	struct stat statbuf;
	stat("result.txt", &statbuf);
	printf("st_ino: %ld\n", statbuf.st_ino);
	printf("st_size: %ld\n", statbuf.st_size);
}

int main(void)
{
    int fd1 = open("result.txt", O_WRONLY, 0777);
    int fd2 = open("result.txt", O_WRONLY, 0777);

    for (char ch = 'a'; ch <= 'z'; ++ch)
    {
        char buf[2];
        snprintf(buf, 2, "%c", ch);
        if (ch % 2) {
            write(fd1, buf, 1);
            file_info();
        }
        else
        {
            write(fd2, buf, 1);
            file_info();
        }
    }

    return 0;
}