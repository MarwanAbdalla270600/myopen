#ifndef MYOPEN_H // Include guard
#define MYOPEN_H

#define READ_END 0
#define WRITE_END 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>


FILE *mypopen(const char *command, const char *mode);

int mypclose(FILE *stream);


#endif