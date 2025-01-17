// proj1.h                                          Stan Eisenstat (09/17/15)
//
// System header files and macros for proj1

#define _GNU_SOURCE
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>

// Write message to stderr using format FORMAT
#define WARN(format, ...) fprintf(stderr, "proj1: " format "\n", __VA_ARGS__)

// Write message to stderr using format FORMAT and exit.
#define DIE(format, ...) WARN(format, __VA_ARGS__), exit(EXIT_FAILURE)

// Double the size of an allocated block PTR with NMEMB members and update
// NMEMB accordingly.  (NMEMB is only the size in bytes if PTR is a char *.)
#define DOUBLE(ptr, nmemb) realloc(ptr, (nmemb *= 2) * sizeof(*ptr))

typedef struct Macro
{
    char *name;
    char *value;
    struct Macro *next; // Okay we're using a linked list, couldn't think of a better way. We got this!
} Macro;

typedef struct Files
{
    int numberOfFiles;
    int numCurrentFile;
    FILE *currentFile;
    char **filenames;

} Files;

int readStream(char *destination, Files *fileStream, int n);
Files *initializeFileStream(char **filenamelist);
bool expandBuffer(char *buffer, Files *filesToRead);
void send(char *s, int b);
void removeAndReplace(char *s, int lenOfRemove, char *replacer);

// function to add stuff to a string
void addToo(char *string, char *add);
bool isValidName(char *s);
char *safeStrdup(char *s);
Macro *defMacro(char *name, char *val, Macro *lastMacro);
Macro *searchMacros(char *name, Macro *starterMacro);