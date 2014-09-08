/*
 * =====================================================================================
 *
 *       Filename:  main.cc
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/27/2014 05:20:12 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  BOSS14420 (), firefoxlinux at gmail dot com
 *   Organization:  
 *
 * =====================================================================================
 */


#include "huffman.hh"
#include <fstream>
#include <vector>
#include <cstring>

int main(int argc, char *argv[]) {

    std::ifstream is(argv[2], std::ios::in | std::ios::binary);
    std::ofstream os(argv[3], std::ios::out | std::ios::binary);

    if (!std::strcmp(argv[1], "c")) {
        std::uint8_t word_length = Huffman::DefaultWordLength; // 16 bit
        
        if (argc >= 5) {
            word_length = std::atoi(argv[4]);
            if (word_length <= 1 || word_length >= 65)
                word_length = Huffman::DefaultWordLength;
        }

        Huffman h(is, Huffman::Compress, word_length);
        h.compress(os);
    } else if (!std::strcmp(argv[1], "x")) {
        Huffman h(is, Huffman::Decompress);
        h.decompress(os);
    }
    is.close();
    os.close();

    return 0;
}
