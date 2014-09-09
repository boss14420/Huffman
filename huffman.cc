/*
 * =====================================================================================
 *
 *       Filename:  huffman.cc
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/03/2014 12:34:11 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  BOSS14420 (), firefoxlinux at gmail dot com
 *   Organization:  
 *
 * =====================================================================================
 */

#include <cstdio>
#include <iostream>
#include <limits>
#include <algorithm>
#include <vector>
#include <deque>
#include <queue>
#include <cstring>

#include "bitstream.hh"
#include "huffman.hh"
#include "integer.hpp"

#define CANNOT_COMPRESS (_min_codelength == _word_length)
#define END_OFFSET (_filesize*8 - _num_words*_word_length - _offset)

const char Huffman::_magic_number[] = {'C', 'O', 'M', 'P',
                                       'R', 'E', 'S', 'S'};

Huffman::Huffman(FILE* is, Action action, std::uint8_t word_length) 
    : _is(is), _action(action), _word_length(word_length)
{
    if (_action == Compress)
        init_compress();
    else if (_action == Decompress)
        init_decompress();
}

void Huffman::compress(FILE *os)
{
    std::cout << "compress ...\n";
    if (_action == Compress) {
        write_header(os);
        encode(os);
    } else {
        throw InvalidAction();
    }
}

void Huffman::decompress(FILE *os)
{
    std::cout << "decompress ...\n";
    if (_action == Decompress) {
        decode(os);
    } else {
        throw InvalidAction();
    }
}


////////////////////////////////////////////////////////////////////////////
//////// Compress methods //////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void Huffman::init_compress()
{
    auto pos = std::ftell(_is);
    std::fseek(_is, 0, SEEK_END);
    _filesize = std::ftell(_is) - pos;
    std::fseek(_is, pos, SEEK_SET);

    _offset = 0;
    _num_words = (_filesize * 8 - _offset) / _word_length;

    std::cout << "calculate_freq_table ...\n";
    calculate_freq_table();
    std::cout << "calculate_code_length ...\n";
    calculate_code_length();
    std::cout << "gen_codewords ...\n";
    gen_codewords();
}

void Huffman::calculate_freq_table()
{
    Word word;
    _freq_table.assign(1 << _word_length, 0);
    _is_pos = std::ftell(_is);
    BitStream ibs(_is, BitStream::Input);
    ibs.read<Word>(_offset); // first extra bits

    std::uint64_t reed_words = 0;
//    while (!ibs.eof()) {
    while (reed_words != _num_words) {
        word = ibs.read<Word>(_word_length);
//    while (!_is.eof()) {
//        _is.read(reinterpret_cast<char*>(&word), _word_length);
        ++_freq_table[word];
        ++reed_words;
    }
    ibs.read<Word>(END_OFFSET); // last extra bits
}

double Huffman::entropy(std::vector<std::size_t> const &_freq_table)
{
    //        std::size_t size = 0;
    double H = 0;
    //        for (auto freq : _freq_table) {
    //            size += _freq_table;
    //
    //        }
    return H;
}

void Huffman::calculate_code_length()
{
    _code_length.assign(_freq_table.size(), 0);
    struct label {
        label() = default;
        label(std::vector<Word> words, std::size_t freq)
            : words(words), freq(freq) {}

        std::vector<Word> words;
        std::size_t freq = 0;
    };

    struct HeapComp {
        bool operator() (label const &l1, label const &l2) {
            return l1.freq >= l2.freq;
        }
    };

    std::priority_queue<label, std::vector<label>, HeapComp> Q;
    for (std::size_t  w = 0; w != ((Word)1 << _word_length); ++w)
        if (_freq_table[w] > 0)
            Q.emplace(std::vector<Word> {static_cast<Word>(w)}, _freq_table[w]);

    while (Q.size() > 1) {
        auto l1 = std::move(Q.top()); Q.pop();
        auto l2 = std::move(Q.top()); Q.pop();
        for (auto w : l1.words) ++_code_length[w];
        for (auto w : l2.words) ++_code_length[w];
        std::vector<Word> words(l1.words.size() + l2.words.size());
        std::copy(l1.words.begin(), l1.words.end(), words.begin());
        std::copy(l2.words.begin(), l2.words.end(), words.begin() + l1.words.size());
        Q.emplace(std::move(words), l1.freq + l2.freq);
    }
}


void Huffman::gen_codewords()
{
    _codewords.assign(_code_length.size(), BitSet());
    auto length_lt = [](CodeLength l1, CodeLength l2) {
        return l1 != 0 && (l2 == 0 || l1 < l2);
    };
    _min_codelength = *std::min_element(_code_length.begin(), _code_length.end(),
                                        length_lt);
    _max_codelength = *std::max_element(_code_length.begin(), _code_length.end());

    std::cout << "min_codelength: " << (int)_min_codelength
              << "\nmax_codelength: " << (int)_max_codelength
              << "\n";

    _header_size = sizeof(_magic_number) + sizeof(_header_size)
                    + sizeof(_filesize) + sizeof(_num_words)
                    + sizeof(_word_length) + sizeof(_min_codelength)
                    + sizeof(_max_codelength);

    if (CANNOT_COMPRESS) return;

    _num_codewords.assign(_max_codelength + 1, 0);
    for (auto l : _code_length) ++_num_codewords[l];

    // _words_string: list all words ordered by code length
    _words_string.reserve(_max_codelength - _min_codelength + 1);
    for (std::size_t w = 0; w != ((Word)1 << _word_length); ++w) 
        if (_code_length[w])
            _words_string.push_back(w);

    auto comp = [this] (Word w1, Word w2) {
        if (_code_length[w1] < _code_length[w2]) return true;
        else if (_code_length[w1] > _code_length[w2]) return false;
        return w1 > w2;
    };
    std::sort(_words_string.begin(), _words_string.end(), comp);

    // max code value of _codewords length i
    _limit.assign(_max_codelength + 1, 0);
    _limit[_min_codelength] = _num_codewords[_min_codelength] - 1;
    for (auto l = _min_codelength + 1; l <= _max_codelength; ++l)
        _limit[l] = (_limit[l-1] + 1) * 2 + (_num_codewords[l] - 1);

    for (auto l = _min_codelength; l <= _max_codelength; ++l) 
        std::cout << (int)l << ", " << (int)_num_codewords[l] << ", " << _limit[l] << '\n';
    std::cout << '\n';
//    std::cout << "words_string: ";
//    for (auto w : _words_string) std::cout << (int) w << ", ";
//    std::cout << '\n';

    auto values = _limit;
    for (auto w : _words_string) {
        auto v = values[_code_length[w]]--;
        _codewords[w] = BitSet(v);
    }

    decltype(_header_size) bits_count = 
        (_max_codelength - _min_codelength + 1) * _word_length
        + _words_string.size() * _word_length + (sizeof(_offset) << 3);
    bits_count = (bits_count == (bits_count / 8) * 8) ? bits_count 
            : (bits_count / 8) * 8 + 8;
    _header_size += bits_count >> 3;
}

/* Huffman::BitSet Huffman::to_bitset(std::size_t value, CodeLength length)
 * {
 *     if (std::floor(std::log2(value)) + 1 > length)
 *         std::cout << value << ", " << (int)length << '\n';
 *     BitSet bitset(length, 0);
 *     auto bi = bitset.rbegin();
 *     while (value) {
 *         *bi++ = (value & 1);
 *         value >>= 1;
 *     }
 *     return bitset;
 * }
 */

void Huffman::encode(FILE* os) const noexcept
{
    std::clearerr(_is);
    std::fseek(_is, _is_pos, SEEK_SET);
    if (!CANNOT_COMPRESS) {
        std::size_t encoded_words = 0;
        Word word;
        BitStream obs(os, BitStream::Output);
        BitStream ibs(_is, BitStream::Input);
        
        word = ibs.read<Word>(_offset);
        obs.write(word, _offset);

        while (encoded_words != _num_words) {
            word = ibs.read<Word>(_word_length);
//            _is.read(reinterpret_cast<char*>(&word), sizeof(Word));
            obs.write(_codewords[word], _code_length[word]);
            ++encoded_words;
        }

        word = ibs.read<Word>(END_OFFSET);
        obs.write(word, END_OFFSET);

        obs.flush();
    } else {
        // copy file
        std::vector<char> buff(4 << 20); // 4MiB buffer
        while (!std::feof(_is)) {
            auto bytes = std::fread(buff.data(), 1, buff.size(), _is);
            std::fwrite(buff.data(), bytes, 1, os);
        }
    }
}

void Huffman::write_header(FILE* os) const
{
    std::fwrite(_magic_number, sizeof(_magic_number), 1, os);

    char bytes[sizeof(_filesize)];
    // write header size
    std::fwrite(int_to_bytes<std::uint32_t>(bytes, _header_size), 4, 1, os);
    // write file size
    std::fwrite(int_to_bytes<std::uint64_t>(bytes, _filesize), 8, 1, os);
    // write num of words
    std::fwrite(int_to_bytes<std::uint64_t>(bytes, _num_words), 8, 1, os);

    std::fwrite(&_word_length, 1, 1, os);
    std::fwrite(&_min_codelength, 1, 1, os);
    std::fwrite(&_max_codelength, 1, 1, os);

    if (!CANNOT_COMPRESS) {
        BitStream obs(os, BitStream::Output);
        for (auto l = _min_codelength; l <= _max_codelength; ++l)
            obs.write(_num_codewords[l], _word_length);

        for (auto w : _words_string)
            obs.write(w, _word_length);
        
        obs.write(_offset, sizeof(_offset) << 3);
        obs.flush();
    }
}


////////////////////////////////////////////////////////////////////////////
//////// Decompress methods ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void Huffman::init_decompress()
{
    std::cout << "read_header ...\n";
    read_header();
}

void Huffman::decode(FILE* os) const noexcept
{
    std::clearerr(_is);
    std::fseek(_is, _header_size, SEEK_SET);

    if (!CANNOT_COMPRESS) {
        BitStream ibs(_is, BitStream::Input);
        BitStream obs(os, BitStream::Output);

        Word extra_bits = ibs.read<Word>(_offset);
        obs.write(extra_bits, _offset);

        CodeLength l = _min_codelength;
        std::size_t value = ibs.read<std::size_t>(_min_codelength);
        decltype(_filesize) _decoded_words = 0;//, readed_bits = _min_codelength;
//        char wordbyte[sizeof(Word)];

        std::vector<Word const*> wsbl(_max_codelength + 1);
        for (auto l = _min_codelength; l <= _max_codelength; ++l)
            wsbl[l] = _words_string.data() + _limit[l] + _base[l];

        while(_decoded_words != _num_words) {
            if (value <= _limit[l]) {
                Word w = *(wsbl[l] - value);
                obs.write(w, _word_length);

                value = ibs.read<decltype(value)>(l = _min_codelength);
                ++_decoded_words;
            } else {
//                CodeLength dl = 0;
//                do { ++dl; } while (!_num_codewords[l+dl]);
//                l += dl;
//                value = (value << dl) | ibs.read<std::size_t>(dl);
                value = (value << 1) | ibs.read<std::uint32_t>(1);
                ++l;
            }
        }

        if (END_OFFSET > l)
            extra_bits = (value << (END_OFFSET - l)) 
                            | ibs.read<Word>(END_OFFSET - l);
        else
            extra_bits = value >> (l - END_OFFSET);
        obs.write(extra_bits, END_OFFSET);
        obs.flush();
    } else {
        // copy file
        std::vector<char> buff(4 << 20); // 4MiB buffer
        while (!std::feof(_is)) {
            auto bytes = std::fread(buff.data(), 1, buff.size(), _is);
            std::fwrite(buff.data(), bytes, 1, os);
        }
    }
}

void Huffman::read_header()
{
    char magic_number[sizeof(_magic_number)];
    std::fread(magic_number, 1, sizeof(_magic_number), _is);
    if (std::strncmp(magic_number, _magic_number, sizeof(_magic_number))) {
        throw InvalidCompressFile();
    }

    char bytes[sizeof(_filesize)];
    std::fread(bytes, 1, sizeof(_header_size), _is);
    _header_size = bytes_to_int<decltype(_header_size)>(bytes);
    std::fread(bytes, 1, sizeof(_filesize), _is);
    _filesize = bytes_to_int<decltype(_filesize)>(bytes);
    std::fread(bytes, 1, sizeof(_num_words), _is);
    _num_words = bytes_to_int<decltype(_num_words)>(bytes);

    std::fread(&_word_length, 1, 1, _is);
    std::fread(&_min_codelength, 1, 1, _is);
    std::fread(&_max_codelength, 1, 1, _is);

    _is_pos = std::ftell(_is);

    if (CANNOT_COMPRESS) return;

    BitStream ibs(_is, BitStream::Input);

    _num_codewords.assign(_max_codelength + 1, 0);
    _limit.assign(_max_codelength + 1, 0);
    _base.assign(_max_codelength + 1, 0);
    auto num_words = 0;
    Word k;

    if (_min_codelength == _max_codelength) {
        _limit[_min_codelength] = (1 << _word_length) - 1;
    }
    for (auto l = _min_codelength; l <= _max_codelength; ++l) {
        k = ibs.read<Word>(_word_length);
        _num_codewords[l] = k;
        num_words += k;

        if (l > _min_codelength) {
            _limit[l] = (_limit[l-1] + 1) * 2 + (k - 1);
            _base[l] = (_base[l-1] + _num_codewords[l-1]);
        } else {
            _limit[l] = k - 1;
        }
    }

    _words_string.assign(num_words, 0);
    for (auto &w : _words_string)
        w = ibs.read<Word>(_word_length);

    _offset = ibs.read<Word>(sizeof(_offset)<<3);

    _is_pos = std::ftell(_is);

//    if (_min_codelength == _max_codelength) return;

    _words.reserve(num_words);
    for (auto l = _min_codelength; l <= _max_codelength; ++l) {
        auto value = _limit[l];
        for (Word k = 0; k != _num_codewords[l]; ++k, --value)
            _words[value] = _words_string[_base[l] + k]; 
    }
}

