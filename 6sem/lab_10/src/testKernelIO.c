#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
    char c;    
    int fd1 = open("alphabet.txt",O_RDONLY);
    int fd2 = open("alphabet.txt",O_RDONLY);
    int flag1 = 1, flag2 = 1;
    while(flag1 == 1 || flag2 == 1)
    {
        if ((flag1 = read(fd1, &c, 1)) == 1)
            printf("%c", c);
        if ((flag2 = read(fd2, &c, 1)) == 1)
            printf("%c", c); 
    }
    puts("");
    return 0;
}