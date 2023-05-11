#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>

#define READ_END 0
#define WRITE_END 1

bool setMode(const char *mode, bool *read, bool *write)
{
    if (mode[0] == 'r' && mode[1] == '\0')
    {
        return *read = true;
    }
    if (mode[0] == 'w' && mode[1] == '\0')
    {
        return *write = true;
    }
    return false;
}

FILE *mypopen(const char *command, const char *mode)
{
    bool read = false;
    bool write = false;

    if (!setMode(mode, &read, &write))
    {
        fprintf(stderr, "pls enter a valid mode\n");
        exit(EXIT_FAILURE);
    }

    int pipefd[2];
    pid_t pid;
    FILE *fp;

    if (pipe(pipefd) == -1)
    {
        return NULL;
    }

    pid = fork();
    if (pid == -1)
    {
        fprintf(stderr, "an error has occured with forking the process");
        return NULL;
    }
    if (pid == 0)
    {
        if (read)
        { // child process reads from pipe
            close(pipefd[READ_END]);
            dup2(pipefd[WRITE_END], STDOUT_FILENO);
            close(pipefd[WRITE_END]);
        }
        else if (write)
        { // child process writes to pipe
            close(pipefd[WRITE_END]);
            dup2(pipefd[READ_END], STDIN_FILENO);
            close(pipefd[READ_END]);
        }

        // execute the command using the shell
        execl("/bin/sh", "sh", "-c", command, NULL);
        _exit(EXIT_FAILURE);
    }
    else
    {
        if (read)
        {
            close(pipefd[WRITE_END]);
            fp = fdopen(pipefd[READ_END], "r");
        }
        else
        {
            close(pipefd[READ_END]);
            fp = fdopen(pipefd[WRITE_END], "w");
        }
        return fp;
    }
}

int mypclose(FILE *stream)
{
    int pid;
    int status;

    if (stream == NULL)
    {
        return -1;
    }

    if (fclose(stream) != 0)
    { /* close the pipe */
        return -1;
    }

    pid = waitpid(-1, &status, 0); /* wait for child process to terminate */
    if (pid == -1)
    {
        return -1;
    }

    if (WIFEXITED(status))
    { /* child process terminated normally */
        return WEXITSTATUS(status);
    }
    else if (WIFSIGNALED(status))
    { /* child process terminated by a signal */
        return 128 + WTERMSIG(status);
    }
    else
    { /* should never happen */
        return -1;
    }
}

int main()
{
    FILE *ls = mypopen("ls", "r");
    FILE *wc = mypopen("wc", "w");
    // we consume the output of `ls` and feed it to `wc`
    char buf[1024];
    while (fgets(buf, sizeof(buf), ls) != NULL)
        fputs(buf, wc);
    // once we're done, we close the streams
    mypclose(ls);
    mypclose(wc);
}