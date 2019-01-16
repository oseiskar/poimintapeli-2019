LINK.o = $(LINK.cc)
CXXFLAGS=-Wall -pedantic -Werror -std=c++11 -O2 -fno-rtti -s
EMCCLOCATION=~/opt/emsdk/emsdk_env.sh
EMCCFLAGS= -std=c++11 -fno-exceptions -DNDEBUG -O2 --memory-init-file 0
#JSUGLIFY=uglifyjs -m -c
JSUGLIFY=cat

.PHONY: clean setup js

default: bin lib bin/main

bin:
	mkdir -p bin

lib:
	mkdir -p lib

js: bin/main.js

bin/main.js: aly.cpp nodemain.cpp main.hpp tila.hpp main.js
	bash -c "source ${EMCCLOCATION} && \
		emcc --bind -o bin/aly.js nodemain.cpp aly.cpp ${EMCCFLAGS} && \
		cat bin/aly.js main.js | \
		${JSUGLIFY} > bin/main.js"

bin/main: lib/aly.o lib/main.o
	$(CXX) $(CXXFLAGS) $^ -o $@

lib/%.o : %.cpp
	$(CXX) $(CXXFLAGS) $^ -c -o $@

clean:
	rm -f *.o lib/*.o bin/*
