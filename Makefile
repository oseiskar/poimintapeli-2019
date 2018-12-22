LINK.o = $(LINK.cc)
CXXFLAGS=-Wall -pedantic -std=c++11

aly: aly.o main.o
aly.o: aly.cpp
main.o: main.cpp

