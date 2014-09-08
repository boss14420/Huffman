/*
 * =====================================================================================
 *
 *       Filename:  bitstream.cc
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/07/2014 09:41:43 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  BOSS14420 (), firefoxlinux at gmail dot com
 *   Organization:  
 *
 * =====================================================================================
 */


#include <cstdint>
#include <iostream>
#include <vector>
#include <memory>
#include <cstring>

#include "integer.hpp"
#include "bitstream.hh"


#define MASK_AND(n) ((ONE << n) - 1)

BitStream::BitStream(std::ios &ios, Type type, std::size_t bufferSize)
    : _ios(ios), _type(type), _bufferSize(bufferSize / nbytes)
{
    _remainingBits = nbits;
    if (_type == Input) {
        _buffer.resize(bufferSize);
        fill_buffer();
        _bufftop = *_bufferindex;
    } else {
        _buffer.reserve(bufferSize);
        _bufftop = 0;
    }
}

/* template<typename T> 
 * T BitStream::read(T count) 
 * {
 *     T ret = 0;
 *     
 *     int diff = (int)count - _remainingBits;
 *     if (count < _remainingBits) {
 *         ret = (_bufftop >> -diff) & MASK_AND(count);
 *         _remainingBits = -diff;
 *     } else if (diff < (int)nbits) {
 *         // add first '_remainingBits' bits
 *         ret = _bufftop & MASK_AND(_remainingBits);
 *         seek_buffer();
 *         // add other 'diff' bits
 *         (ret <<= diff) |= (_bufftop >> (nbits - diff)) & MASK_AND(diff);
 *         _remainingBits = nbits - diff;
 *     } else {
 *         // add first '_remainingBits' bits
 *         ret = _bufftop & MASK_AND(_remainingBits);
 * 
 *         seek_buffer();
 *         while (diff >= (int)nbits) {
 *             (ret <<= nbits) |= _bufftop;
 *             diff -= nbits;
 *             seek_buffer();
 *         }
 * 
 *         // add last 'diff' bits
 *         (ret <<= diff) |= (_bufftop >> (nbits - diff)) & MASK_AND(diff);                         
 *         _remainingBits = nbits - diff;
 *     }
 * 
 *     return ret;
 * }
 * 
 * template BitStream::Int 
 * BitStream::read<BitStream::Int>(BitStream::Int);
 */


// write 'count' LSBs
BitStream& BitStream::write(Int value, int count)
{
    if (!count) return *this;

    int diff = count - _remainingBits;
    if (diff < 0) {
        _bufftop |= (value & MASK_AND(count)) << -diff;
        _remainingBits = -diff;
    } else {
        _bufftop |= (value >> diff) & MASK_AND(_remainingBits);
        _buffer.push_back(_bufftop);
        if (_buffer.size() == _bufferSize)
            flush(false);
        _bufftop = (value & MASK_AND(diff)) << (nbits - diff);
        _remainingBits = nbits - diff;
    }
    return *this;
}

template <typename Iterator>
BitStream& BitStream::write(Iterator first, Iterator last)
{
    if (first == last) return *this;

    Int bufftop = _bufftop;
    auto remain = _remainingBits;

    while (first != last) {
        while (first != last && remain) {
            bufftop |= *first++ << --remain;
        }

        if (!remain) {
            _buffer.push_back(bufftop);
            bufftop = 0;
            remain = BitStream::nbits;
        }

        if (_buffer.size() == _bufferSize)
            flush(false);
    }
    _remainingBits = remain;
    _bufftop = bufftop;

    return *this;
}

void BitStream::flush(bool writeExtraBits)
{
    auto &os = dynamic_cast<std::ostream&>(_ios);
    auto p = std::get_temporary_buffer<char>(_buffer.size() * nbytes);
    auto ptr = p.first;
    for (auto buff : _buffer) {
        int_to_bytes(ptr, buff);
        ptr += nbytes;
    }
    os.write(p.first, p.second);
    std::return_temporary_buffer(p.first);

    // TODO: seek iterator to begin
    _buffer.clear();

    if (writeExtraBits) {
        if (_remainingBits < nbits) {
            int nwrite = nbytes - (_remainingBits / 8);
            char bytes[nbytes];
            os.write(int_to_bytes(bytes, _bufftop), nwrite);
            _bufftop = 0;
            _remainingBits = nbits;
        }
    }
}

void BitStream::fill_buffer()
{
    auto &is = dynamic_cast<std::istream&>(_ios); 
    auto p = std::get_temporary_buffer<char>(_bufferSize * nbytes);
    char *ptr = p.first;

    is.read(ptr, p.second);
    auto count = is.gcount();
    auto round = (count / nbytes) * nbytes;
    char *ptr_end = ptr + ((round == count) ? round : round + nbytes);
    // set extra bytes to zero
    std::memset(ptr + count, 0, ptr_end - (ptr + count));

    auto bi = _buffer.begin();
    for (; ptr < ptr_end; ptr += nbytes, ++bi) {
        *bi = bytes_to_int<Int>(ptr);
    }

    _bufferSize = (ptr_end - p.first) / nbytes;
    _buffer.resize(_bufferSize);
    _bufferindex = _buffer.begin();

    std::return_temporary_buffer(p.first);
//        _eof = .eof();
}

#undef MASK_AND
