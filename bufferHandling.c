#include "bufferHandling.h"
#include "proj1.h"

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