CXX=clang
CXXFLAGS=-std=c++17 -Werror -Wsign-conversion -g
VALGRIND_FLAGS=-v --leak-check=full --show-leak-kinds=all  --error-exitcode=99

SOURCES=ttt.cpp
CODE_SOURCES=ttt.cpp
OBJECTS=$(subst .cpp,.o,$(SOURCES))

all: ttt

run: ttt
	./$^

ttt: ttt.o
	$(CXX) $(CXXFLAGS) $^ -o $@ -lstdc++ -lm

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
	rm -f *.o ttt