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
#include <memory>
#include <cstring>
#include "integer.hpp"

class InBitStream
{
    typedef std::uint64_t Int;
    std::vector<Int> _buffer;
    std::size_t _remainingBits;
    std::istream &_is;
    std::size_t _bufferSize;
    std::size_t _mask;
    std::size_t _bufftop;
    typename std::vector<Int>::iterator _bufferindex;

    static const std::size_t nbytes = sizeof(Int);
    static const std::size_t nbits = nbytes << 3;
    static const std::size_t ONE = 1;

#define MASK_AND(n) ((ONE << n) - 1)

public:
    InBitStream(std::istream &is, std::size_t bufferSize = 4 << 20) 
        : _is(is), _bufferSize(bufferSize / nbytes)
    {
        _mask = ONE << (nbits - 1);
        _remainingBits = nbits;
        _buffer.resize(bufferSize);
        fill_buffer();
//        _is.read(reinterpret_cast<char*>(_buffer.data()), bufferSize);
//        _bufferSize = _is.gcount();
//        _buffer.resize(bufferSize);
//        _bufferindex = _buffer.begin();
        _bufftop = *_bufferindex;
    }

    bool read_1bit() {
        bool bit = _bufftop & _mask;
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
            ret = (_bufftop >> -diff) & MASK_AND(count);
            _remainingBits = -diff;
        } else if (diff < (int)nbits) {
            // add first '_remainingBits' bits
            ret = _bufftop & MASK_AND(_remainingBits);
            seek_buffer();
            // add other 'diff' bits
            (ret <<= diff) |= (_bufftop >> (nbits - diff)) & MASK_AND(diff);
            _remainingBits = nbits - diff;
        } else {
            // add first '_remainingBits' bits
            ret = _bufftop & MASK_AND(_remainingBits);

            seek_buffer();
            while (diff >= (int)nbits) {
                (ret <<= nbits) |= _bufftop;
                diff -= nbits;
                seek_buffer();
            }

            // add last 'diff' bits
            (ret <<= diff) |= (_bufftop >> (nbits - diff)) & MASK_AND(diff);                         
            _remainingBits = nbits - diff;
        }

        _mask = ONE << (_remainingBits - 1);
        
        return ret;
    }

    void seek_buffer() {
        if (++_bufferindex == _buffer.end())
            fill_buffer();
        _bufftop = *_bufferindex;
    }

    void fill_buffer()
    {
//        _is.read(reinterpret_cast<char*>(_buffer.data()), _buffer.size());
        
        auto p = std::get_temporary_buffer<char>(_bufferSize / nbytes);
        char *ptr = p.first;

        _is.read(ptr, p.second);
        auto readed = _is.gcount();
        auto rounded = (readed / nbytes) * nbytes;
        char *ptr_end = ptr + ((rounded == readed) ? rounded : rounded + nbytes);
        // set extra bytes to zero
        std::memset(ptr + rounded, 0, readed - rounded);

        auto bi = _buffer.begin();
        for (; ptr < ptr_end; ptr += nbytes, ++bi) {
            *bi = bytes_to_int<Int>(ptr);
        }

        _bufferSize = (ptr_end - ptr) / nbytes;
        _buffer.resize(_bufferSize);
        _bufferindex = _buffer.begin();
//        _eof = _is.eof();
    }
    
//    void flush(bool writeAll = true)
//    {
//        _os.write(reinterpret_cast<char const*>(_buffer.data()), 
//                  (_buffer.size() - 1) * nbytes);
//
//        auto buff = _buffer.back();
//        _buffer.clear();
//
//        if (writeAll) {
//            if (_remainingBits < nbits)
//                _os.write(reinterpret_cast<char const*>(&buff), nbytes);
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
