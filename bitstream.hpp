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
#include <memory>

#include "integer.hpp"

//template <typename Int = std::uint8_t>
class BitStream
{
    typedef std::uint64_t Int;
    std::vector<Int> _buffer;
    unsigned _remainingBits;
    std::ostream &_os;
    std::size_t _bufferSize;
    Int _bufftop;

    static const std::size_t nbytes = sizeof(Int);
    static const std::size_t nbits = nbytes << 3;
    static const Int ONE = 1;

#define MASK_AND(n) ((ONE << n) - 1)

public:
    BitStream(std::ostream &os, std::size_t bufferSize = 4 << 20) 
        : _os(os), _bufferSize(bufferSize / nbytes)
    {
        _buffer.reserve(_bufferSize);
//        _buffer.push_back(0);
        _bufftop = 0;
        _remainingBits = nbits;
    }

    // write 'count' LSBs
    BitStream& write(Int value, int count)
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
    BitStream& write(Iterator first, Iterator last)
    {
        if (first == last) return *this;

//        Int bufftop = stream._buffer.back();     
        Int bufftop = _bufftop;
//        stream._buffer.pop_back();
        auto remain = _remainingBits;

        while (first != last) {
            while (first != last && remain) {
                bufftop |= *first++ << --remain;
            }
//            stream._buffer.push_back(bufftop);

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

    template <typename BitSet>
    friend 
    BitStream& operator<< (BitStream& stream, BitSet const &bitset)
    {
/*         if (bitset.empty()) return stream;
 * 
 * //        Int bufftop = stream._buffer.back();     
 *         Int bufftop = stream._bufftop;
 * //        stream._buffer.pop_back();
 *         auto remain = stream._remainingBits;
 *         auto bi = bitset.begin();
 * 
 *         while (bi != bitset.end()) {
 *             while (bi != bitset.end() && remain) {
 *                 bufftop |= *bi++ << --remain;
 *             }
 * //            stream._buffer.push_back(bufftop);
 * 
 *             if (!remain) {
 *                 stream._buffer.push_back(bufftop);
 *                 bufftop = 0;
 *                 remain = BitStream::nbits;
 *             }
 * 
 *             if (stream._buffer.size() == stream._bufferSize)
 *                 stream.flush(false);
 *         }
 *         stream._remainingBits = remain;
 *         stream._bufftop = bufftop;
 * 
 *         return stream;
 */
        return stream.write(bitset.begin(), bitset.end());
    }

    void flush(bool writeExtraBits = true)
    {
//        char bytes[nbytes];
//        for (auto buff : _buffer) {
//            _os.write(int_to_bytes(bytes, buff), nbytes);
//        }
        auto p = std::get_temporary_buffer<char>(_buffer.size() * nbytes);
        auto ptr = p.first;
        for (auto buff : _buffer) {
            int_to_bytes(ptr, buff);
            ptr += nbytes;
        }
        _os.write(p.first, p.second);
        std::return_temporary_buffer(p.first);

//        _os.write(reinterpret_cast<char const*>(_buffer.data()), 
//                  _buffer.size() * nbytes);

        // TODO: seek iterator to begin
        _buffer.clear();

        if (writeExtraBits) {
            if (_remainingBits < nbits) {
//                _os.write(reinterpret_cast<char const*>(&_bufftop), nbytes);
                int nwrite = nbytes - (_remainingBits / 8);
                char bytes[nbytes];
                _os.write(int_to_bytes(bytes, _bufftop), nwrite);
                _bufftop = 0;
                _remainingBits = nbits;
            }
        }
    }

    std::size_t bit_count() const {
        return nbits * _buffer.size() + (nbits - _remainingBits);
    }

    std::size_t remaining_bits() const { return _remainingBits; }

#undef MASK_AND
};

#endif // __BITSTREAM_HPP__
