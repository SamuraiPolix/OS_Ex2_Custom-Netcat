CC = gcc
CFLAGS = -Wall -g
VALGRIND_FLAGS = -v --leak-check=full --show-leak-kinds=all  --error-exitcode=99
GCOV = -fprofile-arcs -ftest-coverage -coverage

SOURCES = mync.c
CODE_SOURCES = mync.c
OBJECTS = $(subst .c,.o,$(SOURCES))

INPUTS_FOLDER = inputs
OUTPUTS_FOLDER = outputs
GCOV_OUTPUTS = gcov_outputs

all: mync

.PHONY: all clean

run: mync
	./$^

mync: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

coverage: mync
	$(CC) $(GCOV) -o mync $(SOURCES)
	
	# Test each possible branch of argument parsing
	nc localhost 5001 < $(INPUTS_FOLDER)/player_win.txt | ./mync -e "../q1/ttt 123456789" -i TCPS5001 || true
	gcov mync.c

tidy:
	clang-tidy $(CODE_SOURCES) -checks=bugprone-*,clang-analyzer-*,cppcoreguidelines-*,performance-*,portability-*,readability-*,-cppcoreguidelines-pro-bounds-pointer-arithmetic,-cppcoreguidelines-owning-memory --warnings-as-errors=-* --

valgrind: mync
	valgrind --tool=memcheck $(VALGRIND_FLAGS) ./mync 2>&1 | { egrep "lost| at " || true; }

clean:
	rm -f *.o mync mync.gcda mync.gcno mync.cpp.gcov