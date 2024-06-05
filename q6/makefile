CC = gcc

CFLAGS = -Wall -g

VALGRIND_FLAGS=-v --leak-check=full --show-leak-kinds=all  --error-exitcode=99

SOURCES=mync.c
CODE_SOURCES=mync.c
OBJECTS=$(subst .c,.o,$(SOURCES))

all: mync

.PHONY: all clean run

run: mync
	./$^

mync: mync.c
	$(CC) $(CFLAGS) -o $@ $^

tidy:
	clang-tidy $(CODE_SOURCES) -checks=bugprone-*,clang-analyzer-*,cppcoreguidelines-*,performance-*,portability-*,readability-*,-cppcoreguidelines-pro-bounds-pointer-arithmetic,-cppcoreguidelines-owning-memory --warnings-as-errors=-* --
	# clang-tidy $(CODE_SOURCES) -checks=bugprone-*,clang-analyzer-*,cppcoreguidelines-*,performance-*,portability-*,-cppcoreguidelines-pro-bounds-pointer-arithmetic,-cppcoreguidelines-owning-memory --warnings-as-errors=-* --

valgrind: mync
	valgrind --tool=memcheck $(VALGRIND_FLAGS) ./mync 2>&1 | { egrep "lost| at " || true; }

clean:
	rm -f mync TCP_Receiver TCP_Sender *.o *.h.gch -r uds/*
	touch uds/socket			# keep 1 socket file