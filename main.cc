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
#include <iostream>
#include <vector>
#include <cstring>
#include <cstdio>

#define HELP_STRING (std::string("Usage: ") + argv[0] + " <command> <infile> <outfile> [wlength]\n" + \
                        "wlength: word length in bits (only in compression mode)\n" + \
                        "<command>:\n c: compress <infile>\n x: decompress <infile>\n help: print this help\n")

int main(int argc, char *argv[]) {

    FILE *is = NULL, *os = NULL;

    int retval = 0;
    if (argc > 1 && !std::strcmp(argv[1], "c")) {
        std::uint8_t word_length = Huffman::DefaultWordLength; // 8 bit
        
        if (argc >= 4) {
            is = std::fopen(argv[2], "rb");
            os = std::fopen(argv[3], "wb");

            if (!is) {
                std::cerr << "Cannot open file " << argv[2] << '\n';
                retval = 1;
                goto end;
            } else if (!os) {
                std::cerr << "Cannot open file " << argv[3] << '\n';
                retval = 1;
                goto end;
            }
        }

        if (argc >= 5) {
            word_length = std::atoi(argv[4]);
            if (word_length <= 1 || word_length >= 24) {
                std::cerr << "Invalid wlength " << word_length
                            << "\nUsage: 2 <= wlength <= 24\n";
                goto end;
                retval = 1;
            }
        }

        try {
            Huffman h(is, Huffman::Compress, word_length);
            h.compress(os);
        } catch (std::exception &e) {
            std::cerr << e.what() << '\n';
            retval = 1;
            goto end;
        }
    } else if (argc > 1 && !std::strcmp(argv[1], "x")) {
        if (argc >= 4) {
            is = std::fopen(argv[2], "rb");
            os = std::fopen(argv[3], "wb");

            if (!is) {
                std::cerr << "Cannot open file " << argv[2] << '\n';
                retval = 1;
                goto end;
            } else if (!os) {
                std::cerr << "Cannot open file " << argv[3] << '\n';
                retval = 1;
                goto end;
            }
        }

        try {
            Huffman h(is, Huffman::Decompress);
            h.decompress(os);
        } catch (std::exception &e) {
            std::cerr << e.what() << '\n';
            retval = 1;
            goto end;
        }
    } else if (argc > 1 && !std::strcmp(argv[1], "help")) {
        std::cout << HELP_STRING << '\n';
    } else {
        std::cerr << "Invalid command\n\n";
        std::cerr << HELP_STRING << '\n';
        retval = 1;
    }

end:
    std::fclose(is);
    std::fclose(os);

    return retval;
}
