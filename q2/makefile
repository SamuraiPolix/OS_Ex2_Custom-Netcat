CXX=clang
CC=g++

CXXFLAGS=-std=c++17 -Werror -Wsign-conversion -g
VALGRIND_FLAGS=-v --leak-check=full --show-leak-kinds=all  --error-exitcode=99
GCOV=-fprofile-arcs -ftest-coverage

SOURCES=mync.cpp
CODE_SOURCES=mync.cpp
OBJECTS=$(subst .cpp,.o,$(SOURCES))

INPUTS_FOLDER = inputs
OUTPUTS_FOLDER = outputs
GCOV_OUTPUTS = gcov_outputs

all: mync

.PHONY: all clean run

run: mync
	./$^

mync: mync.o
	$(CXX) $(CXXFLAGS) $^ -o $@ -lstdc++

coverage: 
	$(CC) $(GCOV) -o mync $(SOURCES)

	./mync -e "../q1/ttt 123456789" < $(INPUTS_FOLDER)/draw.txt > $(OUTPUTS_FOLDER)/temp3.txt || true
	./mync -e "../q1/ttt 123456789" < $(INPUTS_FOLDER)/ai_win.txt > $(OUTPUTS_FOLDER)/temp.txt || true
	./mync -e "../q1/ttt 123456789" < $(INPUTS_FOLDER)/player_win.txt > $(OUTPUTS_FOLDER)/temp.txt || true
	./mync -e "../q1/ttt 123456789" < $(INPUTS_FOLDER)/invalid_input.txt > $(OUTPUTS_FOLDER)/temp2.txt || true
	./mync -e "../q1/ttt12345678" < $(INPUTS_FOLDER)/invalid_input.txt > $(OUTPUTS_FOLDER)/temp2.txt || true
	./mync -e "../q1/ttt000  12345678" < $(INPUTS_FOLDER)/invalid_input.txt > $(OUTPUTS_FOLDER)/temp2.txt || true
	gcov mync.cpp

	./mync  - e " . ./  q1 /t tt  1234 56789 " < $(INPUTS_FOLDER)/invalid_input.txt > $(OUTPUTS_FOLDER)/temp2.txt || true
	gcov mync.cpp
tidy:
	clang-tidy $(CODE_SOURCES) -checks=bugprone-*,clang-analyzer-*,cppcoreguidelines-*,performance-*,portability-*,readability-*,-cppcoreguidelines-pro-bounds-pointer-arithmetic,-cppcoreguidelines-owning-memory --warnings-as-errors=-* --
	# clang-tidy $(CODE_SOURCES) -checks=bugprone-*,clang-analyzer-*,cppcoreguidelines-*,performance-*,portability-*,-cppcoreguidelines-pro-bounds-pointer-arithmetic,-cppcoreguidelines-owning-memory --warnings-as-errors=-* --

valgrind: mync
	valgrind --tool=memcheck $(VALGRIND_FLAGS) ./mync 2>&1 | { egrep "lost| at " || true; }

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# mync.o: mync.cpp
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o mync mync.gcda mync.gcno mync.cpp.gcov limits.gcov