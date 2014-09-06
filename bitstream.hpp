/*
 * =====================================================================================
 *
 *       Filename:  bitstream.hpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/27/2014 04:37:55 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  BOSS14420 (), firefoxlinux at gmail dot com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef __BITSTREAM_HPP__
#define __BITSTREAM_HPP__

#include <cstdint>
#include <iostream>
#include <vector>

template <typename Int = std::uint8_t>
class BitStream
{
    std::vector<Int> _buffer;
    std::size_t _remainingBits;
    std::ostream &_os;
    std::size_t _bufferSize;

    static const std::size_t nbits = sizeof(Int) << 3;

public:
    BitStream(std::ostream &os, std::size_t bufferSize = 4 << 20) 
        : _os(os), _bufferSize(bufferSize)
    {
        _buffer.reserve(bufferSize+1);
        _buffer.push_back(0);
        _remainingBits = nbits;
    }

    template <typename BitSet>
    friend 
    BitStream& operator<< (BitStream& stream, BitSet const &bitset)
    {
        if (bitset.empty()) return stream;

        Int buff = stream._buffer.back();     
        stream._buffer.pop_back();
        auto remain = stream._remainingBits;
        auto bi = bitset.begin();

        while (bi != bitset.end()) {
            while (bi != bitset.end() && remain) {
                buff |= *bi++ << --remain;
            }
            stream._buffer.push_back(buff);

            if (bi != bitset.end()) {
                buff = 0;
                remain = BitStream::nbits;
            } else if (!remain) {
                buff = 0;
                remain = BitStream::nbits;
                stream._buffer.push_back(buff);
            }
            if (stream._buffer.size() == stream._bufferSize)
                stream.flush(false);
        }
        stream._remainingBits = remain;

        return stream;
    }

    void flush(bool writeAll = true)
    {
        _os.write(reinterpret_cast<char const*>(_buffer.data()), 
                  (_buffer.size() - 1) * sizeof(Int));

        auto buff = _buffer.back();
        _buffer.clear();

        if (writeAll) {
            if (_remainingBits < nbits)
                _os.write(reinterpret_cast<char const*>(&buff), sizeof(Int));
            _buffer.push_back(0);
            _remainingBits = nbits;
        } else {
            _buffer.push_back(buff);
        }
    }

    std::size_t bit_count() const {
        return nbits * (_buffer.size() - 1) + (nbits - _remainingBits);
    }

    std::size_t remaining_bits() const { return _remainingBits; }
};

#endif // __BITSTREAM_HPP__