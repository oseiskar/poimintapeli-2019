LINK.o = $(LINK.cc)
CXXFLAGS=-Wall -pedantic -std=c++11 -O2 -fno-rtti -s
EMCCFLAGS= -std=c++11 -fno-exceptions -DNDEBUG -O2 --memory-init-file 0
VASTUSTAJAT=lib/vastustaja-greedy.o lib/vastustaja-ei-huom-vast.o lib/vastustaja-vaista-vastustajia.o lib/vastustaja-suorat-reitit.o

.PHONY: clean setup all match show js

default: bin lib bin/main

bin:
	mkdir -p bin

lib:
	mkdir -p lib

js: bin/main.js bin/ugly.js

bin/main.js: aly.cpp nodemain.cpp main.hpp tila.hpp main.js
	bash -c "source ~/opt/emsdk/emsdk_env.sh && emcc --bind -o bin/aly.js nodemain.cpp aly.cpp ${EMCCFLAGS}"
	cat bin/aly.js main.js > bin/main.js

bin/ugly.js: bin/main.js
	uglifyjs -m -c < bin/main.js > bin/ugly.js

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

match: default bin/match
	./bin/match

show: default bin/show_match
	./bin/show_match

clean:
	rm -f *.o lib/*.o bin/*
