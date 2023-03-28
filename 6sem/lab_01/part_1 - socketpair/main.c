#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

int main()
{
  const int cnt_childs = 3;
  const int msg_maxl = 4;
  const char *msg_pr[3] = {
    "aaa",
    "bbb",
    "ccc"
  };
  const char *msg_ch[3] = {
    "111",
    "222",
    "333"
  };
  int socket[2];

  char buf[5];
  buf[4] = '\0';
  pid_t pid[cnt_childs];

  if (socketpair(AF_UNIX, SOCK_DGRAM, 0, socket) == -1) {
    perror("cant socketpair");
    exit(1);
  }

  for (int i = 0; i < cnt_childs; i++)
  {
    if ((pid[i] = fork()) == -1)
    {
      perror("cant fork");
      exit(1);
    }
    if (pid[i] == 0)
    {
      printf("child %d write: %s\n", getpid(), msg_pr[i]);
      write(socket[1], msg_pr[i], msg_maxl);
      read(socket[1], buf, msg_maxl);
      printf("child %d recieve: %s\n", getpid(), buf);
      return 0;
    }
    else {
      read(socket[0], buf, msg_maxl);
      printf("parent recieve: %s from child %d\n", buf, pid[i]);
      write(socket[0], msg_ch[i], msg_maxl);
    printf("parent write: %s\n", msg_ch[i]);
    }
  }

  return 0;
}
