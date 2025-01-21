#include "proj1.h"
Macro *generalParser(Buffer *buffer, Files *filestream, bool inAfter, int parsing, State state, Macro *firstMacro);
Files *initializeFileStream(char **filenamelist, int lengthOfList)
{
    if (lengthOfList < 1)
    {
        WARN("Invalid initialization of file stream%s", "");
    }
    Files *myStuff = (Files *)malloc(sizeof(Files));
    myStuff->numberOfFiles = lengthOfList;
    myStuff->currentFile = fopen(filenamelist[0], "r");
    myStuff->filenames = filenamelist;
    myStuff->numCurrentFile = 0;
    return myStuff;
}
void cleanupFiles(Files *files)
{
    if (files != NULL)
    {
        if (files->currentFile != NULL)
        {
            fclose(files->currentFile);
        }
        free(files);
    }
}
void cleanupMacro(Macro *macro)
{
    if (macro == NULL)
    {
        return;
    }
    cleanupMacro(macro->next);
    free(macro->name);
    free(macro->value);
    free(macro);
}
void cleanupBuffer(Buffer *buffer)
{
    free(buffer->data);
    free(buffer);
}
// this initializes from a char, it is imperfect but functions for now
Buffer *initializeBuffer(Files *fileStream)
{
    Buffer *buffer = (Buffer *)malloc(sizeof(Buffer));
    char *string = (char *)malloc(sizeof(char) * 10);
    int i = fread(string, sizeof(char), 9, fileStream->currentFile);
    buffer->alocatedSize = 10;
    buffer->data = string;
    buffer->sizeOfData = i;
    return buffer;
}
// This returns false if you reach the end of the filestream
bool expandBuffer(Buffer *buffer, Files *fileStream, int n)
{
    if (buffer == NULL || fileStream == NULL)
    {
        DIE("bad inputs to ExpandBuffer: buffer or files are note initialized%s", "");
    }
    int leftToRead = n;
    int requiredSize = buffer->sizeOfData + n + 1; // +1 for null-termination
    if (requiredSize > buffer->alocatedSize)
    {
        int newCapacity = buffer->alocatedSize;
        while (newCapacity < requiredSize)
        {
            newCapacity *= 2;
        }

        buffer->data = realloc(buffer->data, newCapacity);
        buffer->alocatedSize = newCapacity;
    }
    char *destination = buffer->data + buffer->sizeOfData;
    int hold;
    while (leftToRead > 0)
    {
        hold = fread(destination, sizeof(char), leftToRead, fileStream->currentFile);
        destination += hold;
        leftToRead -= hold;

        if (leftToRead > 0)
        {
            if (fileStream->numCurrentFile < fileStream->numberOfFiles - 1)
            {
                fclose(fileStream->currentFile);
                fileStream->numCurrentFile += 1;
                fileStream->currentFile = fopen(fileStream->filenames[fileStream->numCurrentFile], "r");
                if (!fileStream->currentFile)
                {
                    DIE("Failed to open the next file in the stream.%s", "");
                }
            }
            else
            {
                break; // No more files to read
            }
        }
    }
    int bytesRead = n - leftToRead;
    // Update the buffer size and null-terminate
    buffer->sizeOfData += bytesRead;
    return (leftToRead == 0);
}
void send(Buffer *buffer, int b)
{
    if (buffer == NULL || b < 0)
    {
        DIE("invalid input to send%s", "");
    }
    int length = buffer->sizeOfData;
    if (b > length)
    {
        WARN("Input to send were strange, %d larger then langth of buffer data", b);
        b = length; // Adjust b to the length of s to avoid undefined behavior, not sure if this makes sense, maybe I should break here?
    }

    // Send the first b characters to stdout
    fwrite(buffer->data, sizeof(char), b, stdout);
    if (b != length)
    {
        memmove(buffer->data, buffer->data + b, length - b);
    }
    buffer->sizeOfData = buffer->sizeOfData - b;
    // Shift the remaining characters to the beginning of the string
}
// this function should take *s and replace the first lenofremove chars and replace them with replacer.
// Note: replacer can be longer or shorter than lenofremove
// Note: we will NOT free replacer here, we will do it in the state machine
int removeAndReplace(Buffer *b, int lenOfRemove, char *replacer, int start, Files *filestream)
{
    if (b == NULL || lenOfRemove < 0 || replacer == NULL)
    {
        DIE("invalid input to removeAndReplace%s", "");
    }
    int replacerLength = strlen(replacer);
    bool success = true;
    if (lenOfRemove > b->sizeOfData - start)
    {
        success = expandBuffer(b, filestream, lenOfRemove - b->sizeOfData + start);
    }
    if (!success)
    {
        DIE("You cannot remove more than there is in the file stream%s", "");
    }
    int newLength = b->sizeOfData - lenOfRemove + replacerLength;
    int holder = b->alocatedSize;

    while (holder < newLength + 1)
    {
        holder *= 2;
    }
    if (holder != b->alocatedSize)
    {
        char *newData = realloc(b->data, holder);
        if (newData == NULL)
        {
            DIE("Memory reallocation failed in removeAndReplace.%s", "");
        }

        // Update the buffer with the new pointer
        b->data = newData;
        b->alocatedSize = holder;
    }
    memmove(b->data + start + replacerLength, b->data + start + lenOfRemove, b->sizeOfData - start - lenOfRemove + 1); // shifts the memobry so that the stuff after the removed bit is in the right spot
    memcpy(b->data + start, replacer, replacerLength);
    b->sizeOfData = b->sizeOfData - lenOfRemove + replacerLength;
    return holder;
}

// buffer->data + start is the first letter of something known to be a name of a macro.
// This function will get that name and return it as a string. Does not check if it is a valid name
char *getName(Buffer *buffer, int start, Files *filestream)
{
    int i = 0;
    while (buffer->data[start + i] != '{')
    {
        bool worked = true;
        i++;
        while (start + i >= buffer->sizeOfData)
        {
            worked = expandBuffer(buffer, filestream, i + 1);
            if (!worked)
            {
                break;
            }
        }
        if (start + i >= buffer->sizeOfData)
        {
            DIE("Incomplete name in file stream%s", "");
        }
    }
    char *holder = (char *)malloc(sizeof(char) * (i + 1));
    memcpy(holder, buffer->data + start, i);
    holder[i] = '\0';
    return holder;
}
// buffer->data + start is the first letter of something known to be an argument of a macro.
// This function will get that argument and returns it as a string
char *getArg(Buffer *buffer, int start, Files *filestream)
{
    int balance = 1;
    int i = 0;
    while (balance != 0)
    {
        bool worked = true;
        while (start + i >= buffer->sizeOfData)
        {
            worked = expandBuffer(buffer, filestream, i + 1);
            if (!worked)
            {
                break;
            }
        }
        if (start + i >= buffer->sizeOfData)
        {
            DIE("Incomplete argument in file stream%s", "");
        }
        if (buffer->data[start + i] == '{')
        {
            balance += 1;
        }
        if (buffer->data[start + i] == '}')
        {
            balance -= 1;
        }
        if (buffer->data[start + i] == '\\')
        {
            i++;
        }
        i++;
    }
    char *holder = (char *)malloc(sizeof(char) * (i));
    memcpy(holder, buffer->data + start, i - 1);
    holder[i - 1] = '\0';
    return holder;
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
void printMacro(Macro *starterMacro)
{
    if (starterMacro == NULL)
    {
        return;
    }
    printf("Name: %s, Value: %s\n", starterMacro->name, starterMacro->value);
    printMacro(starterMacro->next);
    return;
}
Macro *removeMacro(char *name, Macro *starterMacro)
{
    if (starterMacro == NULL)
    {
        DIE("You cannot undef %s", name);
    }
    if (strcmp(starterMacro->name, name) == 0)
    {
        Macro *macro = starterMacro->next;
        free(starterMacro->name);
        free(starterMacro->value);
        free(starterMacro);
        return macro;
    }
    if (starterMacro->next == NULL)
    {
        DIE("You cannot undef %s", name);
    }
    if (strcmp(starterMacro->next->name, name) == 0)
    {
        Macro *macro = starterMacro->next;
        starterMacro->next = macro->next;
        macro->next = NULL;
        cleanupMacro(macro);
        return starterMacro;
    }
    return removeMacro(name, starterMacro->next);
}
bool isValidName(char *s)
{
    if (strcmp(s, "") == 0)
    {
        DIE("An empty string is not a valid name%s", "");
    }
    while (*s)
    {
        if (!isalnum(*s))
        {
            DIE("Invalid macro name: %s", s);
            return false; // Not alphanumeric
        }
        s++;
    }
    return true;
}
Macro *initializeMacro(char *name, char *val, Macro *firstMacro)
{
    if (searchMacros(name, firstMacro) != NULL)
    {
        DIE("cannot redefine %s", name);
    }
    if (name == NULL || val == NULL)
    {
        DIE("defMacro received a NULL parameter: %s or %s", name, val);
    }
    isValidName(name);
    Macro *madeMacro = (Macro *)malloc(sizeof(Macro));
    // define the macro's stuff
    madeMacro->name = strdup(name);
    madeMacro->value = strdup(val);
    madeMacro->next = NULL;
    if (firstMacro != NULL)
    {
        Macro *holder = firstMacro;
        while (holder->next != NULL)
        {
            holder = holder->next;
        }

        // alocate that memory!

        holder->next = madeMacro;
    }
    return madeMacro;
}

// this takes a buffer, a filestream, and a pointer to the start of a macro known to be def specificly
// it will parse the argument, build the macro needed, and remove the macro. Note start points at d,
// not at the backslash
// TODO: this is broken in some way, relating specifically to it's call to remove and replace.
Macro *parseDef(Buffer *buffer, int start, Files *filestream, Macro *firstMacro)
{
    int startOfArg1 = start + 4;
    char *name = getArg(buffer, startOfArg1, filestream);
    char *value = getArg(buffer, startOfArg1 + strlen(name) + 2, filestream);
    isValidName(name);
    Macro *hold = initializeMacro(name, value, firstMacro);
    if (firstMacro == NULL)
    {
        firstMacro = hold;
    }
    int toberemoved = 8 + strlen(name) + strlen(value);
    if (buffer->sizeOfData <= toberemoved)
    {
        expandBuffer(buffer, filestream, toberemoved - buffer->sizeOfData + 1);
    }
    for (int i = start; i < buffer->sizeOfData; i++)
    {
        printf("%c", buffer->data[i]);
    }
    printf("\n");
    removeAndReplace(buffer, toberemoved, "", start - 1, filestream);
    for (int i = start; i < buffer->sizeOfData; i++)
    {
        printf("%c", buffer->data[i]);
    }
    free(name);
    free(value);
    return firstMacro;
}
// this takes a buffer, a filestream, and a pointer to the start of a macro known to be user defined.
// It will also take the macro that it is. It will parse the argument, and change the text to be as it should be.
void parseUserDefinedMacro(Buffer *buffer, int start, Files *filestream, Macro *macro)
{
    char *name = macro->name;
    int nameLength = strlen(name);
    int startOfArg = start + nameLength + 1;
    char *plugin = getArg(buffer, startOfArg, filestream);
    char *value = macro->value;
    removeAndReplace(buffer, 1 + nameLength + 2 + strlen(plugin), value, start - 1, filestream);
    int numberOfReplacements = 0;
    int valueLen = strlen(value);
    for (int i = 0; i < valueLen; i++)
    {
        if (value[i] == '#')
        {
            removeAndReplace(buffer, 1, plugin, start - 1 + i + numberOfReplacements * (strlen(plugin) - 1), filestream);
            numberOfReplacements++;
        }
    }
    free(plugin);
}
// this takes a buffer, a filestream, and a pointer to the start of a macro known to be undef specificly
// it will parse the argument, undefine the macro, and remove the undef macro. It will throw an error if the name
// is not currenlty a macro. Note start points at u, not at the backslash.

Macro *parseUndef(Buffer *buffer, int start, Files *filestream, Macro *firstMacro) // TOOD: this is broken, specifically when there is only one thing
{
    int startOfArg = start + 6;
    char *name = getArg(buffer, startOfArg, filestream);
    firstMacro = removeMacro(name, firstMacro);
    int toberemoved = 8 + strlen(name);
    if (buffer->sizeOfData <= toberemoved)
    {
        expandBuffer(buffer, filestream, toberemoved - buffer->sizeOfData + 1);
    }
    removeAndReplace(buffer, toberemoved, "", start - 1, filestream);

    free(name);
    return firstMacro;
}
// this takes a buffer, a filestream, and a pointer to the start of a macro known to be if specificly
// it will parse the macro.

void parseIf(Buffer *buffer, int start, Files *filestream)
{
    char *condition = getArg(buffer, start + 3, filestream);
    char *then = getArg(buffer, start + 3 + strlen(condition) + 2, filestream);
    char *otherwise = getArg(buffer, start + 3 + strlen(condition) + 2 + strlen(then) + 2, filestream);
    int sizeOfRemove = 1 + start + 3 + strlen(condition) + 2 + strlen(then) + 2 + strlen(otherwise);
    if (strcmp(condition, "") != 0)
    {
        while (sizeOfRemove >= buffer->sizeOfData - start)
        {
            int i = 2;
            expandBuffer(buffer, filestream, i);
            i++;
        }
        removeAndReplace(buffer, sizeOfRemove, then, start - 1, filestream);
    }
    else
    {
        removeAndReplace(buffer, sizeOfRemove, otherwise, start - 1, filestream);
    }
    free(condition);
    free(then);
    free(otherwise);
}

void parseIfDef(Buffer *buffer, int start, Files *filestream, Macro *startermacro)
{
    char *condition = getArg(buffer, start + 6, filestream);
    char *then = getArg(buffer, start + 6 + strlen(condition) + 2, filestream);
    char *otherwise = getArg(buffer, start + 6 + strlen(condition) + 2 + strlen(then) + 2, filestream);
    int sizeOfRemove = 1 + start + 6 + strlen(condition) + 2 + strlen(then) + 2 + strlen(otherwise);
    isValidName(condition);
    Macro *macro = searchMacros(condition, startermacro);
    if (macro != NULL)
    {
        while (sizeOfRemove >= buffer->sizeOfData - start)
        {
            int i = 2;
            expandBuffer(buffer, filestream, i);
            i++;
        }
        removeAndReplace(buffer, sizeOfRemove, then, start - 1, filestream);
    }
    else
    {
        removeAndReplace(buffer, sizeOfRemove, otherwise, start - 1, filestream);
    }
    free(condition);
    free(then);
    free(otherwise);
}

void parseInclude(Buffer *b, int start, Files *filestream)
{
    char *path = getArg(b, start + 8, filestream);
    char *nameOfIndluce[] = {path};
    Files *littlefilestream = initializeFileStream(nameOfIndluce, 1);
    Buffer *littleBuffer = initializeBuffer(littlefilestream);
    int i = 10;
    bool notFinished = expandBuffer(littleBuffer, littlefilestream, i);
    while (notFinished)
    {
        notFinished = expandBuffer(littleBuffer, littlefilestream, i);
        i = i * 2;
    }
    char *includethis = (char *)malloc(sizeof(char) * littleBuffer->sizeOfData + 1);
    for (int j = 0; j < littleBuffer->sizeOfData; j++)
    {
        includethis[j] = littleBuffer->data[j];
    }
    includethis[littleBuffer->sizeOfData] = '\0';
    removeAndReplace(b, 10 + strlen(path), includethis, start - 1, filestream);
    free(path);
    cleanupBuffer(littleBuffer);
    cleanupFiles(littlefilestream);
    free(includethis);
}
// TODO: this while function lmao
Macro *parseAfter(Buffer *buffer, Files *filestream, int start, Macro *firstMacro)
{
    Buffer *littleBuffer = (Buffer *)malloc(sizeof(Buffer));
    char *before = getArg(buffer, start + 12, filestream);
    char *after = getArg(buffer, start + 12 + strlen(before) + 2, filestream);
    littleBuffer->data = after;
    littleBuffer->sizeOfData = strlen(after);
    littleBuffer->alocatedSize = strlen(after);
    Macro *newMacroList = generalParser(littleBuffer, filestream, true, 0, NORMAL, firstMacro);
    removeAndReplace(buffer, 13 + 2 + 1 + strlen(before) + strlen(littleBuffer->data), before, start - 1, filestream);
    removeAndReplace(buffer, 0, littleBuffer->data, start - 1 + strlen(before), filestream);
    cleanupBuffer(littleBuffer);
    free(before);
    return newMacroList;
}
Macro *generalParser(Buffer *buffer, Files *filestream, bool inAfter, int parsing, State state, Macro *firstMacro)
{
    if (parsing >= buffer->sizeOfData && !inAfter)
    {
        bool unfinished = expandBuffer(buffer, filestream, 1);
        if (!unfinished)
        {
            return firstMacro;
        }
    }
    if (parsing >= buffer->sizeOfData && inAfter)
    {
        return firstMacro;
    }

    switch (state)
    {
    case NORMAL:
        switch (buffer->data[parsing])
        {
        case '%':
            removeAndReplace(buffer, 1, "", parsing, filestream);
            return generalParser(buffer, filestream, inAfter, parsing, COMMENT, firstMacro);
            break;
        case '\\':
            return generalParser(buffer, filestream, inAfter, parsing + 1, BACKSLASH, firstMacro);

            break;
        default:
            if (!inAfter)
            {
                send(buffer, 1); // ERROR? Is there a world where you are outside of a macro but havne't sent it when we aren't in beforeafter? I don't think so, but this is a possible error spot
                return generalParser(buffer, filestream, inAfter, 0, NORMAL, firstMacro);
            }
            else
            {
                return generalParser(buffer, filestream, inAfter, parsing + 1, NORMAL, firstMacro);
            }
            break;
        }
        break;
    case COMMENT:
        switch (buffer->data[parsing])
        {
        case '\n':
            removeAndReplace(buffer, 1, "", parsing, filestream);
            return generalParser(buffer, filestream, inAfter, parsing, NORMAL, firstMacro);
            break;
        default:
            removeAndReplace(buffer, 1, "", parsing, filestream);
            return generalParser(buffer, filestream, inAfter, parsing, NORMAL, firstMacro);
            break;
        }
        break;
    case BACKSLASH:
        if (isalnum(buffer->data[parsing]))
        {
            return generalParser(buffer, filestream, inAfter, parsing, MACRO_NAME, firstMacro);
        }
        else if (buffer->data[parsing] == '\\' || buffer->data[parsing] == '%' || buffer->data[parsing] == '{' || buffer->data[parsing] == '}' || buffer->data[parsing] == '#')
        {
            removeAndReplace(buffer, 1, "", parsing - 1, filestream);
            return generalParser(buffer, filestream, inAfter, parsing, NORMAL, firstMacro);
        }
        else if (inAfter)
        {
            return generalParser(buffer, filestream, inAfter, parsing + 1, NORMAL, firstMacro);
        }
        else
        {
            send(buffer, 2);
            return generalParser(buffer, filestream, inAfter, 0, NORMAL, firstMacro); // whenever anything gets sent, we should be at zero.
        }
        break;
    case MACRO_NAME:
    {
        char *name = getName(buffer, parsing, filestream);
        isValidName(name);
        if (strcmp(name, "def") == 0)
        {
            firstMacro = parseDef(buffer, parsing, filestream, firstMacro);
        }
        else if (strcmp(name, "undef") == 0)
        {
            firstMacro = parseUndef(buffer, parsing, filestream, firstMacro);
        }
        else if (strcmp(name, "if") == 0)
        {
            parseIf(buffer, parsing, filestream);
        }
        else if (strcmp(name, "ifdef") == 0)
        {
            parseIfDef(buffer, parsing, filestream, firstMacro);
        }
        else if (strcmp(name, "include") == 0)
        {
            parseInclude(buffer, parsing, filestream);
        }
        else if (strcmp(name, "expandafter") == 0)
        {
            parseAfter(buffer, filestream, parsing, firstMacro);
        }
        else
        {
            Macro *userDefedMacro = searchMacros(name, firstMacro);
            parseUserDefinedMacro(buffer, parsing, filestream, userDefedMacro);
        }
        free(name);
        return generalParser(buffer, filestream, inAfter, parsing - 1, NORMAL, firstMacro);
        break;
    }
    default:
        return firstMacro;
        break;
    }
}

char *test_filenames[] = {"testFile.txt", "testfile2.txt", "testfile3.txt"};
char *test_filenames2[] = {"testFile.txt"};
void testInitializeFileStream()
{
    Files *files = initializeFileStream(test_filenames, 3);
    cleanupFiles(files);
}
void expandBufferTest1()
{
    printf("### Buffer Test 1: \n");
    Files *myStuff = initializeFileStream(test_filenames, 3);
    Buffer *buffer = initializeBuffer(myStuff);
    expandBuffer(buffer, myStuff, 10);
    for (int i = 0; i < buffer->sizeOfData; i++)
    {
        printf("%c", buffer->data[i]);
    }
    printf("\n");
    cleanupFiles(myStuff);
    free(buffer->data); // Free the string
    free(buffer);
}
void sendtest1()
{
    printf("### Send Test 1: \n");
    char *this = (char *)malloc(sizeof(char) * 11);
    strcpy(this, "testpassed");
    Buffer *buffer = (Buffer *)malloc(sizeof(Buffer));
    buffer->alocatedSize = 11;
    buffer->data = this;
    buffer->sizeOfData = 10;
    send(buffer, 4);
    printf(" ");
    printf("%s\n", this);
    free(buffer->data);
    free(buffer);
}
void testRemoveAndReplace1()
{
    printf("### Remove and Replace test 1: \n");
    char *this = (char *)malloc(sizeof(char) * 11);
    strcpy(this, "testpassed");
    Files *filestream = initializeFileStream(test_filenames, 3);
    Buffer *buffer = (Buffer *)malloc(sizeof(Buffer));
    buffer->alocatedSize = 11;
    buffer->data = this;
    buffer->sizeOfData = 10;
    char *string = (char *)malloc(sizeof(char) * 10);
    strcpy(string, "$$");
    removeAndReplace(buffer, 1, string, 0, filestream);
    send(buffer, buffer->sizeOfData);
    printf("\n");
    cleanupFiles(filestream);
    free(buffer->data);
    free(buffer);
    free(string);
}
void testRemoveAndReplace2()
{
    printf("### Remove and Replace test 2: \n");
    char *this = (char *)malloc(sizeof(char) * 11);
    strcpy(this, "testpassed");
    Files *filestream = initializeFileStream(test_filenames, 3);
    Buffer *buffer = (Buffer *)malloc(sizeof(Buffer));
    buffer->alocatedSize = 11;
    buffer->data = this;
    buffer->sizeOfData = 10;
    char *string = (char *)malloc(sizeof(char) * 20);
    strcpy(string, "bigbigbigbig");
    removeAndReplace(buffer, 1, string, 15, filestream);
    send(buffer, buffer->sizeOfData);
    printf("\n");
    cleanupFiles(filestream);
    free(buffer->data);
    free(buffer);
    free(string);
}
void getNametest()
{
    printf("### getName Test 1: \n");
    Files *filestream = initializeFileStream(test_filenames, 3);
    Buffer *b = initializeBuffer(filestream);
    char *this = getName(b, 1, filestream);
    printf("%s\n", this);
    free(this);
    free(b->data);
    free(b);
    cleanupFiles(filestream);
}
void getArgtest()
{
    printf("### getarg Test: \n");
    Files *filestream = initializeFileStream(test_filenames, 3);
    Buffer *b = initializeBuffer(filestream);
    char *this = getArg(b, 6, filestream);
    printf("%s\n", this);
    free(this);
    free(b->data);
    free(b);
    cleanupFiles(filestream);
}
void searchMacroTest()
{
    printf("### searchmacro Test: \n");

    Macro *macro = (Macro *)malloc(sizeof(Macro));
    char *name = (char *)malloc(sizeof(char) * 10);
    strcpy(name, "name");
    char *name2 = (char *)malloc(sizeof(char) * 10);
    strcpy(name2, "name2");
    char *value = (char *)malloc(sizeof(char) * 10);
    strcpy(value, "value");
    char *value2 = (char *)malloc(sizeof(char) * 10);
    strcpy(value2, "value2");
    macro->name = name;
    macro->value = value;
    macro->next = NULL;
    Macro *macro2 = (Macro *)malloc(sizeof(Macro));
    macro2->name = name2;
    macro2->value = value2;
    macro2->next = macro;

    printf("%d,%d\n", searchMacros("name", macro2) == NULL, searchMacros("notname", macro2) == NULL);
    cleanupMacro(macro2);
}
void expandBufferTest2()
{
    printf("### Buffer Test 2: \n");
    Files *myStuff = initializeFileStream(test_filenames2, 1);
    Buffer *buffer = initializeBuffer(myStuff);
    bool this = expandBuffer(buffer, myStuff, 10);
    for (int i = 0; i < buffer->sizeOfData; i++)
    {
        printf("%c", buffer->data[i]);
    }
    printf("\n");
    printf("%d\n", this);
    cleanupFiles(myStuff);
    free(buffer->data); // Free the string
    free(buffer);
}
void initializeMacroTest()
{
    printf("### initializeMacro Test: \n");
    Macro *tester = initializeMacro("name", "value", NULL);
    printf("%s,%s\n", tester->name, tester->value);
    cleanupMacro(tester);
}
void defTest()
{
    printf("### defTest Test: \n");
    Files *filestream = initializeFileStream(test_filenames, 3);
    Buffer *b = initializeBuffer(filestream);
    Macro *macro = initializeMacro("name2", "value", NULL);
    parseDef(b, 1, filestream, macro);
    printf("%s\n", macro->next->name);
    printf("%s\n", macro->next->value);
    send(b, b->sizeOfData);
    printf("\n");
    cleanupFiles(filestream);
    cleanupBuffer(b);
    cleanupMacro(macro);
}
void testRemoveAndReplace3()
{
    printf("### Remove and Replace test 3,removing it all: \n");
    Files *filestream = initializeFileStream(test_filenames, 3);
    Buffer *buffer = initializeBuffer(filestream);
    send(buffer, buffer->sizeOfData);
    removeAndReplace(buffer, buffer->sizeOfData, "", 0, filestream);
    send(buffer, buffer->sizeOfData);
    printf("\n");
    cleanupBuffer(buffer);
    cleanupFiles(filestream);
}
void testUserDefParser()
{
    printf("### User Def Parser Test 1: \n");
    Files *filestream = initializeFileStream(test_filenames, 3);
    Buffer *buffer = initializeBuffer(filestream);
    Macro *macro = initializeMacro("name", "value # # # # #  ", NULL);
    parseUserDefinedMacro(buffer, 1, filestream, macro);
    send(buffer, buffer->sizeOfData);
    printf("\n");
    cleanupMacro(macro);
    cleanupBuffer(buffer);
    cleanupFiles(filestream);
}
void testUndef()
{
    printf("### Undef Parser Test 1: \n");
    Files *filestream = initializeFileStream(test_filenames, 3);
    Buffer *buffer = initializeBuffer(filestream);
    Macro *macro = initializeMacro("dont", "matter", NULL);
    macro = parseUndef(buffer, 1, filestream, macro);
    expandBuffer(buffer, filestream, 10);
    send(buffer, buffer->sizeOfData);
    printf("\n");
    printMacro(macro);
    cleanupBuffer(buffer);
    cleanupMacro(macro);
    cleanupFiles(filestream);
}
void testIf()
{
    printf("### If Parser Test 1: \n");
    Files *filestream = initializeFileStream(test_filenames, 3);
    Buffer *buffer = initializeBuffer(filestream);
    parseIf(buffer, 1, filestream);
    send(buffer, buffer->sizeOfData);
    printf("\n");
    cleanupBuffer(buffer);
    cleanupFiles(filestream);
}
void testIfDef()
{
    printf("### If Parser Test 1: \n");
    Files *filestream = initializeFileStream(test_filenames, 3);
    Buffer *buffer = initializeBuffer(filestream);
    Macro *macro = initializeMacro("name", "value", NULL);
    parseIfDef(buffer, 1, filestream, macro);
    send(buffer, buffer->sizeOfData);
    printf("\n");
    cleanupMacro(macro);
    cleanupBuffer(buffer);
    cleanupFiles(filestream);
}
void testInclude()
{
    printf("### Include Test 1: \n");
    Files *filestream = initializeFileStream(test_filenames, 3);
    Buffer *buffer = initializeBuffer(filestream);
    parseInclude(buffer, 1, filestream);
    send(buffer, buffer->sizeOfData);
    printf("\n");
    cleanupBuffer(buffer);
    cleanupFiles(filestream);
}
void testExpandAfter()
{
    printf("### Expand After Test 1: \n");
    Files *filestream = initializeFileStream(test_filenames2, 1);
    Buffer *buffer = initializeBuffer(filestream);
    parseAfter(buffer, filestream, 1, NULL);
    send(buffer, buffer->sizeOfData);
    printf("\n");
    cleanupBuffer(buffer);
    cleanupFiles(filestream);
}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        /*testInitializeFileStream();
        expandBufferTest1();
        expandBufferTest2();
        sendtest1();
        testRemoveAndReplace1();
        getNametest();
        //  getArgtest();
        searchMacroTest();
        initializeMacroTest();
        testRemoveAndReplace2();
        testRemoveAndReplace3();
        // defTest();
        // testUserDefParser();
        // testIf();
        // testIfDef();
        // testInclude();*/
        // testExpandAfter();
        testUndef();
    }
    else
    {
        char **filenames = (char **)malloc(sizeof(char *) * (argc - 1));
        for (int i = 0; i < argc - 1; i++)
        {
            filenames[i] = argv[i + 1];
        }
        Files *filestream = initializeFileStream(filenames, argc - 1);
        Buffer *buffer = initializeBuffer(filestream);
        expandBuffer(buffer, filestream, 10);
        Macro *firstMacro = generalParser(buffer, filestream, false, 0, NORMAL, NULL);
        printf("\n");
        cleanupMacro(firstMacro);
        cleanupFiles(filestream);
        free(buffer->data);
        free(buffer);
        free(filenames);
    }
}
