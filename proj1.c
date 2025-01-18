#include "proj1.h"

// this function reads the next n chars out of the stream IF IT CAN if it can't it says how many it did read.
int readStream(char *destination, Files *fileStream, int n)
{
    int lefttoread = n;
    int hold;
    while (lefttoread > 0)
    {
        hold = fread(destination, sizeof(char), lefttoread, fileStream->currentFile);
        destination += hold;
        lefttoread -= hold;
        if (lefttoread != 0 && fileStream->numCurrentFile < fileStream->numberOfFiles - 1)
        {
            fclose(fileStream->currentFile);
            fileStream->numCurrentFile += 1;
            fileStream->currentFile = fopen(fileStream->filenames[fileStream->numCurrentFile], "r");
        }
        else if (lefttoread != 0)
        {
            return n - lefttoread;
        }
    }
    return n - lefttoread;
}

// creates a filestream given a list of file names.
Files *initializeFileStream(char **filenamelist)
{
    size_t count = 0;
    while (filenamelist[count] != NULL)
    {
        count++;
    }
    if (count < 1)
    {
        WARN("Invalid initialization of file stream%s", "");
    }
    Files *myStuff = (Files *)malloc(sizeof(Files)); // lots of error handling needed, make sure argc is at least 2 and stuff
    myStuff->numberOfFiles = count;
    myStuff->currentFile = fopen(filenamelist[0], "r");
    myStuff->filenames = filenamelist;
    myStuff->numCurrentFile = 0;
    return myStuff;
}

// We should double what is in the buffer, doubling it's memory if need be.
// returns negative upon completion of the file stream. otherwise, returns the buffer's new capacity
Buffer *expandBuffer(Buffer *data, Files *filesToRead)
{
    if (data == NULL || filesToRead == NULL)
    {
        DIE("bad inputs to ExpandBuffer: %s or files are NULL", "");
    }
    char *buffer = data->data;
    int bufferSize = data->alocatedSize;
    size_t capacity = bufferSize;
    size_t currentLength = data->sizeOfData;
    size_t requiredLength = currentLength * 2; // Double the current length

    // Check if the current capacity is enough
    if (requiredLength + 1 > capacity) // don't forget null termination :)
    {
        capacity = capacity * 2;
        char *newBuffer = realloc(buffer, capacity * sizeof(char));
        if (newBuffer == NULL)
        {
            // Reallocation failed, can this happen, idk but I keep getting warnings so maybe this'll fix it
            DIE("Failed to alocate memory in expandbuffer %s", "");
        }

        data->data = newBuffer;
    }

    // Read 'currentLength' characters from the file stream into the buffer starting at currentLength
    int bytesRead = readStream(data->data + currentLength, filesToRead, currentLength);
    if (bytesRead < currentLength)
    {
        return -1 * capacity;
    }
    return capacity;
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
    if (firstMacro == NULL)
    {
    }
    Macro *madeMacro = (Macro *)malloc(sizeof(Macro));
    // define the macro's stuff
    madeMacro->name = safeStrdup(name);
    madeMacro->value = safeStrdup(val);
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
int expandMacro(char *input, Macro *first, int inputsize) // we have checked, this is in fact pointing at the right thing, yay!
{
}
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

void send_test_1()
{
    printf("### Send Test 1: \n");
    char *this = (char *)malloc(sizeof(char *) * 6);
    strcpy(this, "testpassed");
    send(this, 4);
    printf(" ");
    printf("%s\n", this);
}
void send_test_2()
{
    printf("### Send Test 2: \n");
    char *this = (char *)malloc(sizeof(char *) * 6);
    strcpy(this, "testpassed");
    send(this, 100);
    printf(" ");
    printf("%s\n", this);
}

char *test_filenames[] = {"testFile.txt", "testfile2.txt", "testfile3.txt"};

void readStreamTest1()
{
    printf("### Stream Test 1: \n");

    Files *myStuff = initializeFileStream(test_filenames);
    char *string = (char *)malloc(sizeof(char) * 10);
    readStream(string, myStuff, 10);
    printf("%s\n", string);
}
void readStreamTest2()
{
    printf("### Stream Test 2: \n");
    Files *myStuff = initializeFileStream(test_filenames);
    char *string = (char *)malloc(sizeof(char) * 500);
    readStream(string, myStuff, 17);
    printf("%s\n", string);
}
void expandBufferTest1()
{
    printf("### Buffer Test 1: \n");
    Files *myStuff = initializeFileStream(test_filenames);
    char *string = (char *)malloc(sizeof(char *) * 3);
    strcpy(string, "hi");
    Buffer *buffer = (Buffer *)malloc(sizeof(Buffer *));
    buffer->alocatedSize = 3;
    buffer->data = string;
    buffer->sizeOfData = 3;
    int success = expandBuffer(buffer, myStuff);
    printf("%s,%d\n", buffer->data, success);
}
void expandBufferTest2()
{
    printf("### Buffer Test 2: \n");
    Files *myStuff = initializeFileStream(test_filenames);
    char *string = (char *)malloc(sizeof(char) * 20);
    strcpy(string, "1234567890123456789");
    Buffer *buffer = (Buffer *)malloc(sizeof(Buffer *));
    buffer->alocatedSize = 20;
    buffer->data = string;
    buffer->sizeOfData = 20;
    int success = expandBuffer(buffer, myStuff);
    printf("%s,%d\n", buffer->data, success);
}
void searchMacrosTest1()
{
    printf("### Search Macros Test 1: \n");
    char *string = (char *)malloc(sizeof(char) * 10);
    strcpy(string, "name");
    char *this = (char *)malloc(sizeof(char) * 10);
    strcpy(this, "value");
    char *string2 = (char *)malloc(sizeof(char) * 10);
    strcpy(string2, "name2");
    char *this2 = (char *)malloc(sizeof(char) * 10);
    strcpy(this2, "value2");
    Macro *hold1 = defMacro(string, this, NULL);
    Macro *hold2 = defMacro(string2, this2, hold1);
    /*printf("Search for something there: %s\n", searchMacros("name2", hold1)->name);
    printf("Search for something not there: %d\n", (NULL == searchMacros("name3", hold1)));
    */
} 
 void testIsValidName()
 {
     printf("### Isvalidname test 1: \n");
     char *string = (char *)malloc(sizeof(char) * 10);
     strcpy(string, "$$");
     char *this = (char *)malloc(sizeof(char) * 10);
     strcpy(this, "value");
     Macro *hold1 = defMacro(string, this, NULL);
     return;
 }
 void testRemoveAndReplace1()
 {
     printf("### Remove and Replace test 1: \n");
     char *string = (char *)malloc(sizeof(char) * 10);
     strcpy(string, "$$");
     char *this = (char *)malloc(sizeof(char) * 10);
     strcpy(this, "value");
     removeAndReplace(this, 1, string, 10);
     printf("%s\n", this);
 }
 void testRemoveAndReplace2()
 {
     printf("### Remove and Replace test 2: \n");
     char *string = (char *)malloc(sizeof(char) * 10);
     strcpy(string, "double");
     char *this = (char *)malloc(sizeof(char) * 10);
     strcpy(this, "value");
     int d = removeAndReplace(this, 1, string, 10);
     printf("%d\n", d);
 }
 void tests()
 {
     send_test_1();
     send_test_2();
     readStreamTest1();
     readStreamTest2();
     expandBufferTest1();
     expandBufferTest2();
     searchMacrosTest1();
     testRemoveAndReplace1();
     testRemoveAndReplace2();
 }

 int main(int argc, char *argv[])
 {
     if (argc = 1)
     {
         tests();
     }
     else
     {
     }
     char *name = (char *)malloc(sizeof(char *) * 10);
     char *val = (char *)malloc(sizeof(char *) * 10);
     strcpy(name, "name");
     strcpy(val, "val");

     Macro *madeMacro = (Macro *)malloc(sizeof(Macro));
     // define the macro's stuff
     madeMacro->name = safeStrdup(name);
     madeMacro->value = safeStrdup(val);
     madeMacro->next = NULL;

     char *text = (char *)malloc(sizeof(char *) * 10);
     strcpy(text, "\\def{}");
     expandMacro(text, madeMacro, 10);
 }