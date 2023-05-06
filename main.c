#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

FILE *
my_popen (const char *cmd)
{
    int fd[2];
    int read_fd, write_fd;
    int pid;               

    /* First, create a pipe and a pair of file descriptors for its both ends */
    if(pipe(fd) == -1) {
        fprintf(stderr, "An error has occured with creating the pipe\n");
        return NULL;
    }
    
    read_fd = fd[0];
    write_fd = fd[1];

    /* Now fork in order to create process from we'll read from */
    pid = fork();

    if(pid == -1) {
        fprintf(stderr, "An error has occured with forking the process\n");
        return NULL;
    }
    
    
    if (pid == 0) {
        /* Child process */

        /* Close "read" endpoint - child will only use write end */
        close(read_fd);

        /* Now "bind" fd 1 (standard output) to our "write" end of pipe */
        dup2(write_fd,1);

        /* Close original descriptor we got from pipe() */
        close(write_fd);

        /* Execute command via shell - this will replace current process */
        execl("/bin/sh", "sh", "-c", cmd, NULL);

        /* Don't let compiler be angry with us */
        return NULL;
    } else {
        /* Parent */

        /* Close "write" end, not needed in this process */
        close(write_fd);

        /* Parent process is simpler - just create FILE* from file descriptor,
           for compatibility with popen() */
        return fdopen(read_fd, "r");
    }
}

int main ()
{
    FILE *p = my_popen ("ls -l");
    char buffer[1024];
    while (fgets(buffer, 1024, p)) {
        printf (" => %s", buffer);
    }
    fclose(p);
}