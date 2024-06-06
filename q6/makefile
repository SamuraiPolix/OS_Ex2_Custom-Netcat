CC = gcc
CFLAGS = -Wall -g
VALGRIND_FLAGS = -v --leak-check=full --show-leak-kinds=all  --error-exitcode=99
GCOV = -fprofile-arcs -ftest-coverage -coverage -lgcov

SOURCES = mync.c
CODE_SOURCES = mync.c
OBJECTS = $(subst .c,.o,$(SOURCES))


all: mync

.PHONY: all clean run

run: mync
	./$^

mync: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

coverage: mync
	$(CC) $(CFLAGS) $(GCOV) -o mync $(SOURCES)

tidy:
	clang-tidy $(CODE_SOURCES) -checks=bugprone-*,clang-analyzer-*,cppcoreguidelines-*,performance-*,portability-*,readability-*,-cppcoreguidelines-pro-bounds-pointer-arithmetic,-cppcoreguidelines-owning-memory --warnings-as-errors=-* --

valgrind: mync
	valgrind --tool=memcheck $(VALGRIND_FLAGS) ./mync 2>&1 | { egrep "lost| at " || true; }

clean:
	rm -f *.o mync
