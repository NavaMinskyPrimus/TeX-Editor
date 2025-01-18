CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Werror

TARGETS = AnotherTry proj1
DEPS = proj1.h

AnotherTry: AnotherTry.c $(DEPS)
	$(CC) $(CFLAGS) -o AnotherTry AnotherTry.c

all: $(TARGETS)

proj1: proj1.c $(DEPS)
	$(CC) $(CFLAGS) -o proj1 proj1.c

clean:
	rm -f $(TARGETS)