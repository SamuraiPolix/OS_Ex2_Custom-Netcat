CXX=clang
CC=g++

CXXFLAGS=-std=c++17 -Werror -Wsign-conversion -g
VALGRIND_FLAGS=-v --leak-check=full --show-leak-kinds=all --error-exitcode=99
GCOV=-fprofile-arcs -ftest-coverage

SOURCES=ttt.cpp
CODE_SOURCES=ttt.cpp
OBJECTS=$(subst .cpp,.o,$(SOURCES))

INPUTS_FOLDER = inputs
OUTPUTS_FOLDER = outputs
GCOV_OUTPUTS = gcov_outputs

all: ttt

.PHONY: all clean run

run: ttt
	./$^

ttt: ttt.o
	$(CXX) $(CXXFLAGS) $^ -o $@ -lstdc++ -lm

coverage: 
	$(CC) $(GCOV) -o ttt $(SOURCES)

	./ttt < $(INPUTS_FOLDER)/invalid_input.txt > $(OUTPUTS_FOLDER)/temp.txt || true
	./ttt 123456788 < $(INPUTS_FOLDER)/invalid_input.txt > $(OUTPUTS_FOLDER)/temp.txt || true
	./ttt 123456789 sa < $(INPUTS_FOLDER)/invalid_input.txt > $(OUTPUTS_FOLDER)/temp.txt || true
	./ttt 12345679 < $(INPUTS_FOLDER)/draw.txt > $(OUTPUTS_FOLDER)/temp3.txt || true
	./ttt 123456789 < $(INPUTS_FOLDER)/ai_win.txt > $(OUTPUTS_FOLDER)/temp.txt || true
	./ttt 123456789 < $(INPUTS_FOLDER)/player_win.txt > $(OUTPUTS_FOLDER)/temp.txt || true
	./ttt 123456789 < $(INPUTS_FOLDER)/invalid_input.txt > $(OUTPUTS_FOLDER)/temp2.txt || true
	gcov ttt.cpp

tidy:
	clang-tidy $(CODE_SOURCES) -checks=bugprone-*,clang-analyzer-*,cppcoreguidelines-*,performance-*,portability-*,readability-*,-cppcoreguidelines-pro-bounds-pointer-arithmetic,-cppcoreguidelines-owning-memory --warnings-as-errors=-* --
	# clang-tidy $(CODE_SOURCES) -checks=bugprone-*,clang-analyzer-*,cppcoreguidelines-*,performance-*,portability-*,-cppcoreguidelines-pro-bounds-pointer-arithmetic,-cppcoreguidelines-owning-memory --warnings-as-errors=-* --

valgrind: ttt
	valgrind --tool=memcheck $(VALGRIND_FLAGS) ./ttt 2>&1 | { egrep "lost| at " || true; }

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ttt.o: ttt.cpp
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o ttt ttt.gcda ttt.gcno ttt.cpp.gcov limits.gcov