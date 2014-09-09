all: hz

CXX 		= g++
#CXXFLAGS 	= -std=c++11 -g -Wall -pipe
CXXFLAGS 	= -std=c++11 -Wall -O3 -march=native -pipe
#CXXFLAGS 	= -std=c++11 -Wall -O3 -march=native -pipe -g -pg

huffman.o: huffman.cc huffman.hh integer.hpp endian.hh bitstream.hh
	$(CXX) $(CXXFLAGS) -c -o $@ $<

bitstream.o: bitstream.cc bitstream.hh integer.hpp endian.hh
	$(CXX) $(CXXFLAGS) -c -o $@ $<

main.o: main.cc huffman.hh
	$(CXX) $(CXXFLAGS) -c -o $@ $<

hz: main.o huffman.o bitstream.o
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -rf *.o
