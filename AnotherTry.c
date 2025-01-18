#include "proj1.h"

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

Buffer *expandBufferByOne(Buffer *buffer, Files *filesToRead)
{
    if (buffer == NULL || filesToRead == NULL)
    {
        DIE("bad inputs to ExpandBuffer: buffer or files are note initialized%s", "");
    }
    int bufferSize = buffer->alocatedSize;
    size_t capacity = bufferSize;
    int currentLength = buffer->sizeOfData;
    size_t requiredLength = currentLength + 2; // Double the current length

    // Check if the current capacity is enough
    if (requiredLength > capacity) // don't forget null termination :)
    {
        capacity = capacity * 2;
        buffer->data = realloc(buffer->data, capacity * sizeof(char));
        if (buffer->data == NULL)
        {
            DIE("Memory reallocation failed.%s", "");
        }
    }
    buffer->alocatedSize = capacity;
    // Read 'currentLength' characters from the file stream into the buffer starting at currentLength
    char *data = buffer->data;
    int bytesRead = readStream(data + currentLength, filesToRead, 1);
    buffer->data = data;
    if (bytesRead < currentLength)
    {
        return buffer;
    }
    return buffer;
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

char *test_filenames[] = {"testFile.txt", "testfile2.txt", "testfile3.txt"};

void testInitializeFileStream()
{
    Files *files = initializeFileStream(test_filenames);
    cleanupFiles(files);
}

void readStreamTest1()
{
    printf("### Stream Test 1: \n");

    Files *myStuff = initializeFileStream(test_filenames);
    char *string = (char *)malloc(sizeof(char) * 11);
    int bytesRead = readStream(string, myStuff, 10);
    string[bytesRead] = '\0';
    printf("%s\n", string);
    cleanupFiles(myStuff);
    free(string);
}

void readStreamTest2()
{
    printf("### Stream Test 2: \n");
    Files *myStuff = initializeFileStream(test_filenames);
    char *string = (char *)malloc(sizeof(char) * 50);
    int hold = readStream(string, myStuff, 17);
    string[hold] = '\0';
    printf("%s\n", string);

    cleanupFiles(myStuff);
    free(string);
}

void expandBufferTest1()
{
    printf("### Buffer Test 1: \n");
    Files *myStuff = initializeFileStream(test_filenames);
    char *string = (char *)malloc(sizeof(char) * 3);
    strcpy(string, "hi");
    Buffer *buffer = (Buffer *)malloc(sizeof(Buffer));
    buffer->alocatedSize = 3;
    buffer->data = string;
    buffer->sizeOfData = 2;
    expandBufferByOne(buffer, myStuff);
    char *stuff = (char *)malloc(sizeof(char) * (buffer->sizeOfData + 1));
    for (int i = 0; i < buffer->sizeOfData; i++)
    {
        stuff[i] = buffer->data[i];
    }
    stuff[buffer->sizeOfData] = '\0';
    printf("%s,%d\n", stuff, buffer->alocatedSize);
    cleanupFiles(myStuff);
    free(buffer->data); // Free the string
    free(buffer);
    free(stuff);
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

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    testInitializeFileStream();
    readStreamTest1();
    readStreamTest2();
    expandBufferTest1();
    send_test_1();
    testRemoveAndReplace1();
}
