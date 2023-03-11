#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#define BUF_SIZE 32

void log_exit(char *msg)
{
  perror(msg);
  exit(1);
}

int main()
{
  const int ch_amount = 3;
  const char *msg_to_pr[3] = {
    "aaaa",
    "bbbb",
    "cccc"
  };
  const char *msg_to_ch[3] = {
    "1111",
    "2222",
    "3333"
  };
  int socketp[2];

  char buf[BUF_SIZE];
  buf[BUF_SIZE - 1] = '\0';
  pid_t pid[ch_amount];

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, socketp) == -1)
      log_exit("cant socketpair");

  for (int i = 0; i < ch_amount; i++)
  {
    if ((pid[i] = fork()) == -1)
      log_exit("cant fork");
    else if (pid[i] == 0)
    {
      close(socketp[0]);
      write(socketp[1], msg_to_pr[i], BUF_SIZE);
      sleep(2*i);
      read(socketp[1], buf, BUF_SIZE);
      printf("child: pid = %d, got msg: %s\n", getpid(), buf);
      return 0;
    }
  }

  for (int i = 0; i < 3; i++) {
    if (pid[i]) {
      close(socketp[1]);
      read(socketp[0], buf, BUF_SIZE);
      printf("parent: ch_pid = %d, got msg: %s\n", pid[i], buf);
      write(socketp[0], msg_to_ch[i], BUF_SIZE);
    }
  }

  return 0;
}