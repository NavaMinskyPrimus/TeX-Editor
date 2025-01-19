CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Werror -O3

TARGETS = AnotherTry
DEPS = proj1.h
TESTS = $(wildcard *.t)

AnotherTry: AnotherTry.c $(DEPS)
	$(CC) $(CFLAGS) -o AnotherTry AnotherTry.c

all: $(TARGETS)

proj1: proj1.c $(DEPS)
	$(CC) $(CFLAGS) -o proj1 proj1.c

clean:
	rm -f $(TARGETS)

test: $(TARGETS)
	cram $(TESTS)

.PHONY: test clean all