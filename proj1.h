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

typedef struct Buffer
{
    int alocatedSize;
    char *data;
    int sizeOfData;
    bool cantExpand;
} Buffer;

typedef enum
{
    NORMAL,
    COMMENT,
    ENDCOMMENT,
    BACKSLASH,
    MACRO_NAME,
    PARSING_ARGS,
} State;
