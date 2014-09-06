/*
 * =====================================================================================
 *
 *       Filename:  inbitstream.hpp
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

#ifndef __INBITSTREAM_HPP__
#define __INBITSTREAM_HPP__

#include <cstdint>
#include <iostream>
#include <vector>

template <typename Int = std::uint8_t>
class InBitStream
{
    std::vector<Int> _buffer;
    std::size_t _remainingBits;
    std::istream &_is;
    std::size_t _bufferSize;
    Int _mask;
    typename std::vector<Int>::iterator _bufferindex;

    static const std::size_t nbits = sizeof(Int) << 3;
    static const std::size_t ONE = 1;

#define MASK_AND(n) ((ONE << n) - 1)

public:
    InBitStream(std::istream &is, std::size_t bufferSize = 4 << 20) 
        : _is(is), _bufferSize(bufferSize)
    {
        _mask = ONE << (nbits - 1);
        _remainingBits = nbits;
        _buffer.resize(bufferSize);
        _is.read(reinterpret_cast<char*>(_buffer.data()), bufferSize);
        _bufferSize = _is.gcount();
        _buffer.resize(bufferSize);
        _bufferindex = _buffer.begin();
    }

    bool read_1bit() {
        bool bit = *_bufferindex & _mask;
        if ( !(--_remainingBits, _mask >>= 1) ) {
            _mask = ONE << (nbits - 1);
            _remainingBits = nbits;
            seek_buffer();
        }
        return bit;
    }

    template<typename T> T read(T count) {
        T ret = 0;
/*         while (count--) {
 *             ret |= read_1bit() << count;
 *         }
 */
        
        int diff = (int)count - _remainingBits;
        if (count < _remainingBits) {
            ret = (*_bufferindex >> -diff) & MASK_AND(count);
            _remainingBits = -diff;
        } else if (diff < (int)nbits) {
            // add first '_remainingBits' bits
            ret = *_bufferindex & MASK_AND(_remainingBits);
            seek_buffer();
            // add other 'diff' bits
            (ret <<= diff) |= (*_bufferindex >> (nbits - diff)) & MASK_AND(diff);
            _remainingBits = nbits - diff;
        } else {
            // add first '_remainingBits' bits
            ret = *_bufferindex & MASK_AND(_remainingBits);

            do {
                seek_buffer();
                (ret <<= nbits) |= *_bufferindex;
                diff -= nbits;
            } while (diff >= (int)nbits);

            if (diff > 0) {
                // add remaining 'diff' bits
                (ret <<= diff) |= (*_bufferindex >> (nbits - diff)) & MASK_AND(diff);                         
                _remainingBits = nbits - diff;
            } else {
                seek_buffer();
                _remainingBits = nbits;
            }
        }

        _mask = ONE << (_remainingBits - 1);
        
        return ret;
    }

    void seek_buffer() {
        if (++_bufferindex == _buffer.end())
            fill_buffer();
    }

    void fill_buffer()
    {
        _is.read(reinterpret_cast<char*>(_buffer.data()), _buffer.size());
        _bufferSize = _is.gcount();
        _buffer.resize(_bufferSize);
        _bufferindex = _buffer.begin();
//        _eof = _is.eof();
    }
    
//    void flush(bool writeAll = true)
//    {
//        _os.write(reinterpret_cast<char const*>(_buffer.data()), 
//                  (_buffer.size() - 1) * sizeof(Int));
//
//        auto buff = _buffer.back();
//        _buffer.clear();
//
//        if (writeAll) {
//            if (_remainingBits < nbits)
//                _os.write(reinterpret_cast<char const*>(&buff), sizeof(Int));
//            _buffer.push_back(0);
//            _remainingBits = nbits;
//        } 
//    }
//
//    std::size_t bit_count() const {
//        return nbits * (_buffer.size() - 1) + (nbits - _remainingBits);
//    }
//
//    std::size_t remaining_bits() const { return _remainingBits; }

#undef MASK_AND
};

#endif // __INBITSTREAM_HPP__
