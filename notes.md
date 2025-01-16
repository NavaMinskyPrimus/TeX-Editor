# Some ideas on how to read files

I think a reasonable way to read files is to write a function that will read from a collection of files as if it were a single stream. I think for that, you need a simple data-structure that tracks where you are in the reading. I'm thinking something like:

- argc and argv
- the number of the currently open file (0 if no file is open)
- the file pointer of the currently open file

and then you just have to implement a single function like this:

read_file_stream(max_chars_to_read,buf_to_write_into,pos_in_buf_to_write_into)

and this function should return an integer which is the number of characters it actually read.a

Internally, the function would try to read the requested number of characters from the current file. If it succeeds, well, great! It just writes those into the buffer. But if it hits the end of the file, then it should close the current file, and open the next file.

There are some details to work out here. There are a few conditions you might need to signal. Like:

- return 0 if you read 0 bytes
- return -1 if you're at the very end of the list of files.a

But then, in the driver code, when you want to read N characters, you just call this function. And you keep on doing that until you have enough characters to do what you need to do next. And you only stop when you finally get -1.

# How to write tests

first off, you need to organize your code into pieces that make it easy to run tests. And that's easier if you separate the bits of logic into their own files, and then have files that pull things together to run them.a

So, for a given thing you want to test, you want to:

- have



Things to do:
1. make a function that initializes a file stream
2. try to make buffer doubler
3. try to make progress on understanding how before after will work and recursion and stuff