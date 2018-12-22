LINK.o = $(LINK.cc)
CXXFLAGS=-Wall -pedantic -std=c++11

.PHONY: clean setup all

all: bin/main

bin/main: lib/aly.o lib/main.o
	$(CXX) $(CXXFLAGS) $^ -o $@

lib/%.o : %.cpp
	$(CXX) $(CXXFLAGS) $^ -c -o $@

clean:
	rm -f *.o vastustajat/*.o bin/*

setup:
	mkdir -p bin lib
