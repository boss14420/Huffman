/*
 * =====================================================================================
 *
 *       Filename:  huffman.hh
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/03/2014 12:39:45 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  BOSS14420 (), firefoxlinux at gmail dot com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef __HUFFMAN_HH__
#define __HUFFMAN_HH__

#include <iostream>
#include <vector>
#include <unordered_map>
#include <bitset>

struct InvalidAction : public std::exception {
    virtual char const *what() const noexcept override {
        return "Invalid action";
    }
};

struct InvalidCompressFile : public std::exception {
    virtual char const *what() const noexcept override {
        return "Invalid compress file";
    }
};

class Huffman {
public:
    typedef std::uint64_t Word;
    typedef std::uint8_t CodeLength;
//    typedef std::vector<bool> BitSet;
//    typedef std::bitset<64> BitSet;
    typedef std::uint64_t BitSet;
    static const char _magic_number[];
    static const std::uint8_t DefaultWordLength = 16;

    enum Action { Compress, Decompress };

private:
    std::istream &_is;
    Action _action;
    std::uint8_t _word_length;

    std::vector<std::size_t> _freq_table;
    std::vector<CodeLength> _code_length;
    CodeLength _min_codelength, _max_codelength;
    std::vector<Word> _num_codewords;
    std::vector<Word> _base;
    std::unordered_map<std::size_t, Word> _words;
    std::vector<Word> _words_string;
    std::vector<std::size_t> _limit;
    std::vector<BitSet> _codewords;
    std::ios::pos_type _is_pos;
    std::uint32_t _header_size;
    std::uint64_t _filesize;
    std::uint64_t _num_words;
    std::uint8_t _offset;

public:
  Huffman(std::istream &is, Action action, 
          std::uint8_t word_length = DefaultWordLength);

    void compress(std::ostream &os);
    void decompress(std::ostream &os);

private:

////////////////////////////////////////////////////////////////////////////
//////// Compress methods //////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

    void init_compress();

    void calculate_freq_table();

    static double entropy(std::vector<std::size_t> const &freq_table);

    void calculate_code_length();

    void gen_codewords();

    static BitSet to_bitset(std::size_t value, CodeLength length);

    void encode(std::ostream& os) const noexcept;

    void write_header(std::ostream& os) const;

////////////////////////////////////////////////////////////////////////////
//////// Compress methods //////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

    void init_decompress();

    void read_header();

    void decode(std::ostream& os) const noexcept;
};

#endif // __HUFFMAN_HH__
