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

void expandBuffer(Buffer *buffer, Files *fileStream, int n) // make this return false if you reach the end of the filestream
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
int removeAndReplace(Buffer *b, int lenOfRemove, char *replacer)
{
    if (b == NULL || lenOfRemove < 0 || replacer == NULL)
    {
        DIE("invalid input to removeAndReplace%s", "");
    }
    int replacerLength = strlen(replacer);
    if (lenOfRemove > b->sizeOfData)
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
    memmove(b->data + replacerLength, b->data + lenOfRemove, b->sizeOfData - lenOfRemove + 1); // shifts the memobry so that the stuff after the removed bit is in the right spot
    memcpy(b->data, replacer, replacerLength);
    return holder;
}

// buffer->data + start is the first letter of something known to be a name of a macro. This function will get that name.
char *getName(Buffer *buffer, int start, Files *filestream)
{
    char *string = buffer->data;
    int i = 0;

    while (string[start + i] != '{')
    {
        if (start + i >= buffer->sizeOfData)
        {
            expandBuffer(buffer, filestream, i); // TODO: deal with end of filestream
            string = buffer->data;
        }
        i++;
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
        if (start + i > buffer->sizeOfData)
        {
            expandBuffer(buffer, filestream, i); // TODO: deal with end of filestream
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
    }
    char *holder = (char *)malloc(sizeof(char) * (i));
    memcpy(holder, buffer->data + start, i - 1);
    holder[i - 1] = '\0';
    return holder;
}

char *test_filenames[] = {"testFile.txt", "testfile2.txt", "testfile3.txt"};
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
    removeAndReplace(buffer, 1, string);
    printf("%s\n", buffer->data);
    free(buffer->data);
    free(buffer);
    free(string);
}
void getNametest()
{
    printf("### getName Test: \n");
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

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        testInitializeFileStream();
        expandBufferTest1();
        send_test_1();
        testRemoveAndReplace1();
        getNametest();
        getArgtest();
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
