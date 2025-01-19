#include "proj1.h"

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

bool expandBuffer(Buffer *buffer, Files *fileStream, int n) // make this return false if you reach the end of the filestream
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
    if (buffer == NULL || b <= 0)
    {
        DIE("invalid input to send%s", "");
    }
    int length = buffer->sizeOfData;
    if (b > length)
    {
        WARN("Input to send were strange, %d larger then langth of %s", b, buffer->data);
        b = length; // Adjust b to the length of s to avoid undefined behavior, not sure if this makes sense, maybe I should break here?
    }

    // Send the first b characters to stdout
    fwrite(buffer->data, sizeof(char), b, stdout);

    // Shift the remaining characters to the beginning of the string
    memmove(buffer->data, buffer->data + b, length - b + 1);
}
// this function should take *s and replace the first lenofremove chars and replace them with replacer.
// Note: replacer can be longer or shorter than lenofremove
// Note: we will NOT free replacer here, we will do it in the state machine
int removeAndReplace(Buffer *b, int lenOfRemove, char *replacer, int start)
{
    if (b == NULL || lenOfRemove < 0 || replacer == NULL)
    {
        DIE("invalid input to removeAndReplace%s", "");
    }
    int replacerLength = strlen(replacer);
    if (lenOfRemove > b->sizeOfData - start)
    {
        WARN("Input to removeandreplace was strange, %d larger then length of data", lenOfRemove);
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
    return holder;
}

// buffer->data + start is the first letter of something known to be a name of a macro.
// This function will get that name and return it as a string
char *getName(Buffer *buffer, int start, Files *filestream)
{
    char *string = buffer->data;
    int i = 0;

    while (string[start + i] != '{')
    {
        if (start + i >= buffer->sizeOfData)
        {
            expandBuffer(buffer, filestream, i);
            string = buffer->data;
        }
        i++;
        if (start + i == buffer->sizeOfData)
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
    while (balance != 0 && i < 10)
    {
        while (start + i >= buffer->sizeOfData)
        {
            expandBuffer(buffer, filestream, i); // TODO: deal with end of filestream
            i++;
        }
        if (buffer->data[start + i] == '{')
        {
            balance += 1;
        }
        if (buffer->data[start + i] == '}')
        {
            balance -= 1;
        }
        i++;
        if (start + i >= buffer->sizeOfData)
        {
            DIE("Incomplete argument in file stream%s", "");
        }
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
bool isValidName(char *s, Macro *starterMacro)
{
    while (*s)
    {
        if (!isalnum(*s))
        {
            DIE("Invalid macro name: %s", s);
            return false; // Not alphanumeric
        }
        s++;
    }
    if (searchMacros(s, starterMacro) != NULL)
    {
        DIE("Cannot re-define %s", s);
        return false;
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
    isValidName(name, firstMacro);
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

void parseDef(Buffer *buffer, int start, Files *filestream, Macro *firstMacro)
{
    int startOfArg1 = start + 4;
    char *name = getArg(buffer, startOfArg1, filestream);
    char *value = getArg(buffer, startOfArg1 + strlen(name) + 1, filestream);
    isValidName(name, firstMacro);
    initializeMacro(name, value, firstMacro);
    removeAndReplace(buffer, 8 + strlen(name) + strlen(value), "", start - 1);
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
void send_test_1()
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
    Buffer *buffer = (Buffer *)malloc(sizeof(Buffer));
    buffer->alocatedSize = 11;
    buffer->data = this;
    buffer->sizeOfData = 10;
    char *string = (char *)malloc(sizeof(char) * 10);
    strcpy(string, "$$");
    removeAndReplace(buffer, 1, string, 0);
    printf("%s\n", buffer->data);
    free(buffer->data);
    free(buffer);
    free(string);
}
void testRemoveAndReplace2()
{
    printf("### Remove and Replace test 2: \n");
    char *this = (char *)malloc(sizeof(char) * 11);
    strcpy(this, "testpassed");
    Buffer *buffer = (Buffer *)malloc(sizeof(Buffer));
    buffer->alocatedSize = 11;
    buffer->data = this;
    buffer->sizeOfData = 10;
    char *string = (char *)malloc(sizeof(char) * 20);
    strcpy(string, "bigbigbigbig");
    removeAndReplace(buffer, 1, string, 5);
    printf("%s\n", buffer->data);
    free(buffer->data);
    free(buffer);
    free(string);
}
void getNametest()
{
    printf("### getName Test 1: \n");
    Files *filestream = initializeFileStream(test_filenames2, 3);
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
void initializeMacroTest()
{
    printf("### initializeMacro Test: \n");
    Macro *tester = initializeMacro("name", "value", NULL);
    printf("%s,%s\n", tester->name, tester->value);
    cleanupMacro(tester);
}
void defTest()
{
    /*
    printf("### defTest Test: \n");
    Files *filestream = initializeFileStream(test_filenames, 3);
    Buffer *b = initializeBuffer(filestream);
    Macro *macro = initializeMacro("name", "value", NULL);
    parseDef(b, 1, filestream, macro);*/
}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        testInitializeFileStream();
        expandBufferTest1();
        send_test_1();
        testRemoveAndReplace1();
        getNametest();
        // getArgtest();
        searchMacroTest();
        initializeMacroTest();
        testRemoveAndReplace2();
        // defTest();
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
        send(buffer, 10);
        cleanupFiles(filestream);
        free(buffer->data);
        free(buffer);
        free(filenames);
    }
}
