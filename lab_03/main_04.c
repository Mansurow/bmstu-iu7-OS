#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define N 2

int main(void)
{
    int fd[N];
    pid_t childpid[N];
    
    if (pipe(fd) == -1)
    {
       perror("Can't pipe.\n");
       exit(1);
    }
        
    char *const message[N] = { "xyz!\n", "xx, yy, zz\n" }; 
 
    for (size_t i = 0; i < N; i++)
    {
    	
        childpid[i] = fork();
	    if (childpid[i] == -1)
	    {
	        perror("Can't fork.\n");
	        exit(1);
	    }
	    else if (childpid[i] == 0)
	    {
	        close(fd[0]);
	        write(fd[1], message[i], strlen(message[i]));
	        printf("Message #%ld sent to parent!\n", i + 1);
	        	            
	        return 0;
	    }
	    else 
	    {
	        printf("Parent process: PID = %d, GROUP = %d, child's ID = %d\n", 
        	    getpid(),
        	    getpgrp(),
        	    childpid[i]
        	    );
        	    
        	    int status;
	        pid_t child_pid = wait(&status);
	        
	        if (child_pid == -1)
	        {
	        	perror("Can't wait.\n");
	        	exit(1);
	        }
	        
	        printf("Child #%ld has finished: PID = %d\n", i + 1, child_pid);
	    
	        if (WIFEXITED(status))
	          printf("Child #%ld exited with code %d\n", i + 1, WEXITSTATUS(status));
	        else if(WIFSIGNALED(status))
	          printf("Child #%ld terminated, recieved signal %d\n", i + 1, WTERMSIG(status));
	        else if (WIFSTOPPED(status))
	          printf("Child #%ld stopped, recieved signal %d\n", i + 1, WSTOPSIG(status));   
        	   
	       close(fd[1]);
           read(fd[0], message[i], sizeof(message[i]));
    
           printf("Received messages: %s\n", message[i]);
	    }
    }
     
    return 0;
}
