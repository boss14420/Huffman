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

#include <iostream>
#include <limits>
#include <algorithm>
#include <vector>
#include <deque>
#include <queue>
#include <cstring>

#include "bitstream.hpp"
#include "inbitstream.hpp"
#include "huffman.hh"
#include "integer.hpp"

const std::size_t Huffman::WordLength = 8;
const char Huffman::_magic_number[] = "COMPRESS";

Huffman::Huffman(std::istream& is, Action action) 
    : _is(is), _action(action)
{
    if (_action == Compress)
        init_compress();
    else if (_action == Decompress)
        init_decompress();
}

void Huffman::compress(std::ostream &os)
{
    std::cout << "compress ...\n";
    if (_action == Compress) {
        write_header(os);
        encode(os);
    } else {
        throw "Invalid action";
    }
}

void Huffman::decompress(std::ostream &os)
{
    std::cout << "decompress ...\n";
    if (_action == Decompress) {
        decode(os);
    } else {
        throw "Invalid action";
    }
}


////////////////////////////////////////////////////////////////////////////
//////// Compress methods //////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void Huffman::init_compress()
{
    auto pos = _is.tellg();
    _is.seekg(0, std::ios::end);
    _filesize = _is.tellg() - pos;
    _is.seekg(pos);

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
    _freq_table.assign(1 << WordLength, 0);
    _is_pos = _is.tellg();
    while (!_is.eof()) {
        _is.read(reinterpret_cast<char*>(&word), sizeof(word));
        ++_freq_table[word];
    }
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
    for (std::size_t  w = 0; w < std::numeric_limits<Word>::max() + 1; ++w)
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

    // TODO: stop if _min_codelength == _max_codelength
//    if (_min_codelength == _max_codelength) return;

    _num_codewords.assign(_max_codelength + 1, 0);
    for (auto l : _code_length) ++_num_codewords[l];

    // _words_string: list all words ordered by code length
    _words_string.reserve(_max_codelength - _min_codelength + 1);
    for (std::size_t w = 0; w < std::numeric_limits<Word>::max() + 1; ++w) 
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
    if (_min_codelength != _max_codelength)
        _limit[_min_codelength] = _num_codewords[_min_codelength] - 1;
    else
        _limit[_min_codelength] = std::numeric_limits<Word>::max();
    for (auto l = _min_codelength + 1; l <= _max_codelength; ++l)
        _limit[l] = (_limit[l-1] + 1) * 2 + (_num_codewords[l] - 1);

    std::cout << "min_codelength: " << (int)_min_codelength
              << "\nmax_codelength: " << (int)_max_codelength
              << "\n";
    for (auto l = _min_codelength; l <= _max_codelength; ++l) 
        std::cout << (int)l << ", " << (int)_num_codewords[l] << ", " << _limit[l] << '\n';
    std::cout << '\n';
    std::cout << "words_string: ";
    for (auto w : _words_string) std::cout << (int) w << ", ";
    std::cout << '\n';

    auto values = _limit;
    for (auto w : _words_string) {
        auto v = values[_code_length[w]]--;
        _codewords[w] = std::move(to_bitset(v, _code_length[w]));
    }
}

Huffman::BitSet Huffman::to_bitset(std::size_t value, CodeLength length)
{
    if (std::floor(std::log2(value)) + 1 > length)
        std::cout << value << ", " << (int)length << '\n';
    BitSet bitset(length, 0);
    auto bi = bitset.rbegin();
    while (value) {
        *bi++ = (value & 1);
        value >>= 1;
    }
    return bitset;
}

void Huffman::encode(std::ostream& os) const
{
    _is.clear();
    _is.seekg(_is_pos);
    if (_min_codelength != _max_codelength) {
        Word word;
        BitStream<> bs(os);
        while (!_is.eof()) {
            _is.read(reinterpret_cast<char*>(&word), sizeof(Word));
            bs << _codewords[word];
        }
        bs.flush();
    } else {
        // copy file
        std::vector<char> buff(4 << 20); // 4MiB buffer
        while (!_is.eof()) {
            _is.read(buff.data(), buff.size());
            auto bytes = _is.gcount(); 
            os.write(buff.data(), bytes);
        }
    }
}

void Huffman::write_header(std::ostream& os) const
{
    os.write(_magic_number, sizeof(_magic_number));
    // TODO: endianess
    char bytes[8];
    // write file size
    os.write(int_to_bytes<std::uint64_t>(bytes, _filesize), 8);
    // write num of words
    os.write(int_to_bytes<std::uint64_t>(bytes, _filesize), 8);

    Word wl = WordLength;
    os.write(reinterpret_cast<char const*>(&wl), sizeof(Word));
    os.write(reinterpret_cast<char const*>(&_min_codelength), sizeof(CodeLength));
    os.write(reinterpret_cast<char const*>(&_max_codelength), sizeof(CodeLength));
    os.write(reinterpret_cast<char const*>(&_num_codewords[_min_codelength]), 
                _max_codelength - _min_codelength + 1);
    os.write(reinterpret_cast<char const*>(_words_string.data()), 
                _words_string.size() * sizeof(Word));
}


////////////////////////////////////////////////////////////////////////////
//////// Decompress methods ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void Huffman::init_decompress()
{
    std::cout << "read_header ...\n";
    read_header();
}

void Huffman::decode(std::ostream& os) const
{
    _is.clear();
    _is.seekg(_is_pos);

    if (_min_codelength != _max_codelength) {
        InBitStream<> ibs(_is);
        CodeLength l = _min_codelength;
        std::size_t value = ibs.read<std::size_t>(_min_codelength);
        decltype(_filesize) _decoded_words = 0;
        char wordbyte[sizeof(Word)];
        while(_decoded_words != _filesize) {
            if (value <= _limit[l]) {
                Word w = _words.find(value)->second;
//                os.write(reinterpret_cast<char const*>(&w), sizeof(Word));
                os.write(int_to_bytes<Word>(wordbyte, w), sizeof(Word));
                value = ibs.read<decltype(value)>(l = _min_codelength);
                ++_decoded_words;
            } else {
                CodeLength dl = 0;
                do { ++dl; } while (!_num_codewords[l+dl]);
                l += dl;
                value = (value << dl) | ibs.read<std::size_t>(dl);
            }
        }
    }
}

void Huffman::read_header()
{
    char magic_number[sizeof(_magic_number)];
    _is.read(magic_number, sizeof(_magic_number));
    if (std::strncmp(magic_number, _magic_number, sizeof(_magic_number))) {
        throw "Invalid compressed file";
    }

    char bytes[sizeof(_filesize)];
    _is.read(bytes, sizeof(_filesize));
    _is.read(bytes, sizeof(_filesize));
    _filesize = bytes_to_int<decltype(_filesize)>(bytes);

    Word word_length;
    _is.read(reinterpret_cast<char*>(&word_length), sizeof(Word));
    _is.read(reinterpret_cast<char*>(&_min_codelength), sizeof(CodeLength));
    _is.read(reinterpret_cast<char*>(&_max_codelength), sizeof(CodeLength));

    _num_codewords.assign(_max_codelength + 1, 0);
    _limit.assign(_max_codelength + 1, 0);
    _base.assign(_max_codelength + 1, 0);
    auto num_words = 0;
    CodeLength k;

    if (_min_codelength == _max_codelength)
        _limit[_min_codelength] = (1 << word_length) - 1;
    for (auto l = _min_codelength; l <= _max_codelength; ++l) {
        _is.read(reinterpret_cast<char*>(&k), sizeof(CodeLength));
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
    _is.read(reinterpret_cast<char*>(_words_string.data()),
                num_words * sizeof(Word));

    _is_pos = _is.tellg();

    if (_min_codelength == _max_codelength) return;

    _words.reserve(num_words);
    for (auto l = _min_codelength; l <= _max_codelength; ++l) {
        auto value = _limit[l];
        for (auto k = 0; k != _num_codewords[l]; ++k, --value)
            _words[value] = _words_string[_base[l] + k]; 
    }
}

