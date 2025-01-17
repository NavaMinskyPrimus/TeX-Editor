#include "proj1.h"
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

// we'll add states if we need them!
typedef enum
{
    NORMAL,
    COMMENT,
    BACKSLASH,
    MACRO_NAME,
    PARSING_ARGS,
    EXPANDING,
    EXPANDAFTER
} State;

char *expandText(char *input)
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
                removeAndReplace(input + i - 1, 2, pass);
                printf("%s\n", input);
            }
            else if (isalnum(firstchar))
            {
                state = MACRO_NAME;
            }
            else
            {
                DIE("Non alpha-numeric macro name element, %c", firstchar);
            }
        case MACRO_NAME:

        default:
            break;
        }
    }
}
/*
void handleExpandAfter(char **arguments, char *outputBuffer)
{
    char *before = arguments[0];
    char *after = arguments[1];

    // Expand 'after' first
    char *expandedAfter = expandText(after);

    // Then expand 'before' with the expanded 'after'
    char *expandedBefore = expandText(before);

    // Output the combined result
    appendToOutput(expandedBefore);
    appendToOutput(expandedAfter);
}
*/
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
    char *string = (char *)malloc(sizeof(char) * 10);
    strcpy(string, "\\\\");

    expandText(string);
}
