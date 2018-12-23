LINK.o = $(LINK.cc)
CXXFLAGS=-Wall -pedantic -std=c++11
VASTUSTAJAT=lib/vastustaja-greedy.o

.PHONY: clean setup all match show

all: bin/main

bin/main: lib/aly.o lib/main.o
	$(CXX) $(CXXFLAGS) $^ -o $@

lib/vastustaja-%.o : vastustajat/%.cpp
	$(CXX) $(CXXFLAGS) $^ -c -o $@

lib/%.o : %.cpp
	$(CXX) $(CXXFLAGS) $^ -c -o $@

bin/match: lib/aly.o match.cpp $(VASTUSTAJAT)
	$(CXX) $(CXXFLAGS) $^ -o $@

bin/show_match: lib/aly.o match.cpp $(VASTUSTAJAT)
	$(CXX) -DSHOW_MATCH=1 $(CXXFLAGS) $^ -o $@

match: bin/match
	./bin/match

show: bin/show_match
	./bin/show_match

clean:
	rm -f *.o lib/*.o bin/*

setup:
	mkdir -p bin lib
