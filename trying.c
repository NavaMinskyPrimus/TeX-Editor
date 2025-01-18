#include "proj1.h"
#include <malloc.h>
int expandAfter(char *input, Macro *first, int inputsize);
int undef(char *input, Macro *first, int inputsize);
int expandIf(char *input, Macro *first, int inputsize);
int ifDef(char *input, Macro *first, int inputsize);
int userDefMacro(char *input, Macro *first, int inputsize, Macro *this);
int expandMacro(char *input, Macro *first, int inputsize);
int def(char *input, Macro *first, int inputsize);
int expandText(char *input, Macro *first, int inputSize);
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
// function to add stuff to a string
int addToo(char *string, char *add, int sSize)
{
    if (string == NULL)
    {
        string = (char *)malloc(strlen(add));
        strcpy(string, add);
        return strlen(add);
    }
    size_t length = strlen(string) + strlen(add);
    size_t holder = sSize;
    while (holder < length + 1)
    {
        holder *= 2;
    }

    if (holder != sSize)
    {
        string = realloc(string, holder);
    }
    memcpy(string + strlen(string), add, strlen(add));
    return holder;
}
bool isValidName(char *s)
{
    while (*s)
    {
        if (!isalnum(*s))
        {
            return false; // Not alphanumeric
        }
        s++;
    }
    return true;
}
Macro *defMacro(char *name, char *val, Macro *firstMacro)
{
    if (searchMacros(name, firstMacro) != NULL)
    {
        DIE("cannot redefine %s", name);
    }
    if (name == NULL || val == NULL)
    {
        DIE("defMacro received a NULL parameter: %s or %s", name, val);
    }
    if (!isValidName(name))
    {
        DIE("Invalid macro name: %s", name);
    }
    Macro *holder = firstMacro;
    while (holder->next != NULL)
    {
        holder = holder->next;
    }
    // alocate that memory!
    Macro *madeMacro = (Macro *)malloc(sizeof(Macro));
    // define the macro's stuff
    madeMacro->name = safeStrdup(name);
    madeMacro->value = safeStrdup(val);
    madeMacro->next = NULL;
    // append macro to linked list. I feel like we're just rfreeing null below, but better safe than sorry.

    holder->next = madeMacro;
    return madeMacro;
}

Macro *searchMacros(char *name, Macro *starterMacro)
{
    if (starterMacro == NULL)
    {
        return NULL;
    }
    if (strcmp(starterMacro->name, name) == 0)
    {
        return starterMacro;
    }
    return searchMacros(name, starterMacro->next);
}

int removeAndReplace(char *s, int lenOfRemove, char *replacer, int sSize)
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
    size_t holder = sSize;

    while (holder < newLength + 1)
    {
        holder *= 2;
    }
    if (holder != sSize)
    {
        s = realloc(s, holder);
    }
    memmove(s + strlen(replacer), s + lenOfRemove, strlen(s) - lenOfRemove + 1); // shifts the memobry so that the stuff after the removed bit is in the right spot
    memcpy(s, replacer, strlen(replacer));
    return holder;
}

// we'll add states if we need them!
typedef enum
{
    NORMAL,
    COMMENT,
    BACKSLASH,
    MACRO_NAME,
    PARSING_ARGS,
} State;

int expandAfter(char *input, Macro *first, int inputsize)
{
}
int undef(char *input, Macro *first, int inputsize)
{
}
int expandIf(char *input, Macro *first, int inputsize)
{
}
int ifDef(char *input, Macro *first, int inputsize)
{
}
int userDefMacro(char *input, Macro *first, int inputsize, Macro *this)
{
}

int def(char *input, Macro *first, int inputsize)
{
    char *name = (char *)malloc((sizeof(char *)) * 10);
    int nameSize = 10;
    char *c = (char *)malloc((sizeof(char *)) * 2);
    for (int i = 5; i < inputsize; i++)
    {
        if (input[i] == '{')
        {
            break;
        }
        if (isalnum(input[i]))
        {
            strcpy(c, &input[i]);
            nameSize = addToo(name, c, nameSize);
        }
        else
        {
            DIE("non-alphanumeric character in macro name, %c", input[i]);
        }
    }
    printf("%s", name);
}
// this takes specifically a macro and finds all it's arguments and expands it
// Note: this will be responsible for checking the macro's name for copmliance
// step 1: identify name
// step 2: go to currect sub-function
int expandMacro(char *input, Macro *first, int inputsize) // we have checked, this is in fact pointing at the right thing, yay!
{
    char *name = (char *)malloc(sizeof(char *) * 10);
    int nameSize = 10;
    char *c = (char *)malloc(sizeof(char *) * 2);
    for (int i = 1; i < strlen(input); i++)
    {
        if (i >= nameSize)
        {
            nameSize = nameSize * 2;
            name = realloc(name, nameSize * sizeof(char));
        }
        if (input[i] == '{')
        {
            i = strlen(input);
        }
        else if (isalnum(input[i]))
        {
            strcpy(c, &input[i]);
            addToo(name, c, nameSize);
        }
        else
        {
            DIE("Non-alphanumeric symbol in macro name, %c", input[i]);
        }
    }
    printf("%s\n", name);
    Macro *this = searchMacros(name, first);
    if (this != NULL)
    {
        userDefMacro(input, first, inputsize, this);
    }
    else if (strcmp(name, "def") == 0)
    {
        def(input, first, inputsize);
    }
    else
    {
        printf("not yet implemenented\n");
    }
}
// this takes any text and expand it
int expandText(char *input, Macro *first, int inputSize)
{
    State state = NORMAL;
    for (int i = 0; i < strlen(input); i++)
    {
        char firstchar = input[i];
        switch (state)
        {
        case NORMAL:
            switch (firstchar)
            {
            case '\\':
                state = BACKSLASH;
                break;
            case '%':
                state = COMMENT;
                break;
            default:
                break;
            }
            break;
        case COMMENT:
            switch (firstchar)
            {
            case '\n':
                state = NORMAL;
                break;
            default:
                break;
            }
        case BACKSLASH:
            if (strchr("#\\{}%", firstchar) != NULL)
            {
                char *pass = (char *)malloc(sizeof(char) * 2);
                pass[0] = firstchar;
                inputSize = removeAndReplace(input + i - 1, 2, pass, inputSize);
            }
            else if (isalnum(firstchar))
            {
                state = MACRO_NAME;
                i -= 1;
            }
            else
            {
                DIE("Non alpha-numeric macro name element, %c", firstchar);
            }
            break;
        case MACRO_NAME:
            int hold = expandMacro(input + i - 1, first, inputSize - i + 1);
            inputSize = hold + i - 1;
            state = NORMAL;
            i -= 1;
            break;
        default:
            break;
        }
    }
}
int main(int argc, char *argv[])
{
    /*State state = NORMAL;
    char currentChar;
    char *macroName = (char *)malloc(sizeof(char *) * 10);
    char **arguments = (char **)malloc(sizeof(char **) * 10);
    int argCount = 0;
    switch (state)
    {
    case EXPANDAFTER:
        break;

    default:
        break;
    }*/
    char *string = (char *)malloc(sizeof(char) * 15);
    strcpy(string, "hi \\def{}{}");
    Macro *madeMacro = (Macro *)malloc(sizeof(Macro));
    // define the macro's stuff
    madeMacro->name = "hello";
    madeMacro->value = "there";
    madeMacro->next = NULL;
    expandText(string, madeMacro, 10);
}
