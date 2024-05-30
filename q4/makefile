CC = gcc

CFLAGS = -Wall -g

VALGRIND_FLAGS=-v --leak-check=full --show-leak-kinds=all  --error-exitcode=99

SOURCES=mync.cpp
CODE_SOURCES=TCP_Sender.cpp, TCP_Receiver.cpp, mync.cpp
OBJECTS=$(subst .cpp,.o,$(SOURCES))

# We don't really need TCP_Sender and TCP_Receiver because we can use nc and telnet to test the server and client
# they are kept here but are not used - they don't match the question's requirements 100%
# all: mync TCP_Sender TCP_Receiver
all: mync

.PHONY: all clean

run: mync
	./$^

mync: mync.c
	$(CC) $(CFLAGS) -o $@ $^

TCP_Receiver: TCP_Receiver.c
	$(CC) $(CFLAGS) -o $@ $^

TCP_Sender: TCP_Sender.c
	$(CC) $(CFLAGS) -o $@ $^

tidy:
	clang-tidy $(CODE_SOURCES) -checks=bugprone-*,clang-analyzer-*,cppcoreguidelines-*,performance-*,portability-*,readability-*,-cppcoreguidelines-pro-bounds-pointer-arithmetic,-cppcoreguidelines-owning-memory --warnings-as-errors=-* --
	# clang-tidy $(CODE_SOURCES) -checks=bugprone-*,clang-analyzer-*,cppcoreguidelines-*,performance-*,portability-*,-cppcoreguidelines-pro-bounds-pointer-arithmetic,-cppcoreguidelines-owning-memory --warnings-as-errors=-* --

valgrind: mync
	valgrind --tool=memcheck $(VALGRIND_FLAGS) ./mync 2>&1 | { egrep "lost| at " || true; }

clean:
	rm -f mync TCP_Receiver TCP_Sender *.o *.h.gch