all: test

CXX 		= g++
#CXXFLAGS 	= -std=c++11 -g -Wall -pipe
CXXFLAGS 	= -std=c++11 -Wall -O3 -march=native -pipe -g

huffman.o: huffman.cc huffman.hh integer.hpp bitstream.hpp inbitstream.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

test.o: main.cc huffman.hh
	$(CXX) $(CXXFLAGS) -c -o $@ $<

test: test.o huffman.o
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -rf *.o
