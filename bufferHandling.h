#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>

void send(char *s, int b);
void removeAndReplace(char *s, int lenOfRemove, char *replacer);

typedef struct Files
{
    int numberOfFiles;
    int numCurrentFile;
    FILE *currentFile;
    char **filenames;

} Files;