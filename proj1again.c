#include "proj1.h"

void readStream()
{
}

// This function takes *s and shifts it over, freeing the first b chars and sending them to stdout. YAY!!
void send(char *s, int b)
{
    if (s == NULL || b <= 0)
    {
        DIE("invalid input to send: %s, %d", s, b);
    }
    size_t length = strlen(s);
    if (b > length)
    {
        WARN("Input to send were strange, %d larger then langth of %s", b, s);
        b = length; // Adjust b to the length of s to avoid undefined behavior, not sure if this makes sense, maybe I should break here?
    }

    // Send the first b characters to stdout
    fwrite(s, sizeof(char), b, stdout);

    // Shift the remaining characters to the beginning of the string
    memmove(s, s + b, length - b + 1);
}

// this function should take *s and replace the first lenofremove chars and replace them with replacer.
// Note: replacer can be longer or shorter than lenofremove
// Note: we will NOT free replacer here, we will do it in the state machine
void removeAndReplace(char *s, int lenOfRemove, char *replacer)
{
    if (s == NULL || lenOfRemove < 0 || replacer == NULL)
    {
        DIE("invalid input to removeAndReplace: %p, %d, %p", (void *)s, lenOfRemove, (void *)replacer);
    }
    size_t sLength = strlen(s);
    size_t replacerLength = strlen(replacer);
    if (lenOfRemove > strlen(s))
    {
        WARN("Input to removeandreplace was strange, %d larger then langth of %s", lenOfRemove, s);
    }
    size_t newLength = sLength - lenOfRemove + replacerLength;
    size_t holder = malloc_usable_size(s);

    while (holder < newLength + 1)
    {
        holder *= 2;
    }
    if (holder != malloc_usable_size(s))
    {
        s = realloc(s, holder);
    }
    memmove(s + strlen(replacer), s + lenOfRemove, strlen(s) - lenOfRemove + 1); // shifts the memobry so that the stuff after the removed bit is in the right spot
    memcpy(s, replacer, strlen(replacer));
}

// function to add stuff to a string
void addToo(char *string, char *add)
{
    if (string == NULL)
    {
        string = (char *)malloc(10);
        strcpy(string, add);
        return;
    }
    size_t length = strlen(string) + strlen(add);
    size_t holder = malloc_usable_size(string);
    while (holder < length + 1)
    {
        holder *= 2;
    }

    if (holder != malloc_usable_size(string))
    {
        string = realloc(string, holder);
    }
    memcpy(string + strlen(string), add, strlen(add));
}
bool isValidName(char *s)
{
    return true;
}
char *safeStrdup(char *s)
{
    if (s == NULL)
    {
        DIE("can't copy %s", s);
    }
    char *dup = strdup(s);
    if (dup == NULL)
    {
        DIE("Failed to allocate memory for %s", "strdup");
    }
    return dup;
}
Macro *defMacro(char *name, char *val, Macro *lastMacro)
{
    if (name == NULL || val == NULL)
    {
        DIE("defMacro received a NULL parameter: %s", "Invalid input");
    }
    if (!isValidName(name))
    {
        DIE("Invalid macro name: %s", name);
    }
    // alocate that memory!
    Macro *madeMacro = (Macro *)malloc(sizeof(Macro));
    // define the macro's stuff
    madeMacro->name = safeStrdup(name);
    madeMacro->value = safeStrdup(val);
    madeMacro->next = NULL;
    // append macro to linked list. I feel like we're just rfreeing null below, but better safe than sorry.
    if (lastMacro != NULL)
    {
        free(lastMacro->next);
        lastMacro->next = madeMacro;
    }
    return madeMacro;
}

int main(int argc, char *argv[])
{
    Files *myStuff = (Files *)malloc(sizeof(Files)); // lots of error handling needed, make sure argc is at least 2 and stuff
    myStuff->numberOfFiles = argc;
    myStuff->currentFile = fopen(argv[1], "r");
    myStuff->filenames = argv;
    myStuff->numCurrentFile = 0;
}