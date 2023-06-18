#include <stdio.h>
#include <sys/stat.h>

void file_info(FILE *fs)
{
	struct stat statbuf;
	stat("result.txt", &statbuf);
	printf("st_ino: %ld\n", statbuf.st_ino);
	printf("st_size: %ld\n", statbuf.st_size);
	printf("pos: %ld\n", ftell(fs));
}

int main(void)
{
    FILE *fs1 = fopen("result.txt", "w");
    file_info(fs1);

    FILE *fs2 = fopen("result.txt", "w");
    file_info(fs2);

    for (char ch = 'a'; ch <= 'z'; ++ch)
        fprintf(ch % 2 ? fs1 : fs2, "%c", ch);

    file_info(fs1);
    fclose(fs1);

    file_info(fs2);
    fclose(fs2);

    return 0;
}