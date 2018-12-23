LINK.o = $(LINK.cc)
CXXFLAGS=-Wall -pedantic -std=c++11

.PHONY: clean setup all match

all: bin/main

bin/main: lib/aly.o lib/main.o
	$(CXX) $(CXXFLAGS) $^ -o $@

bin/match: lib/aly.o match.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

lib/%.o : %.cpp
	$(CXX) $(CXXFLAGS) $^ -c -o $@

match: bin/match
	./bin/match

clean:
	rm -f *.o lib/*.o bin/*

setup:
	mkdir -p bin lib
