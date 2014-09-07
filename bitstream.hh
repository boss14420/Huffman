/*
 * =====================================================================================
 *
 *       Filename:  bitstream.hh
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/07/2014 09:38:24 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  BOSS14420 (), firefoxlinux at gmail dot com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef __BITSTREAM_HH__
#define __BITSTREAM_HH__

#include <cstdint>
#include <iosfwd>
#include <vector>

#include "integer.hpp"

//template <typename Int = std::uint8_t>
class BitStream
{
public:
    typedef std::uint64_t Int;
    enum Type { Input, Output };

private:
    std::vector<Int> _buffer;
    unsigned _remainingBits;
    std::ios &_ios;
    Type _type;
    std::size_t _bufferSize;
    Int _bufftop;
    typename std::vector<Int>::iterator _bufferindex;

    static const std::size_t nbytes = sizeof(Int);
    static const std::size_t nbits = nbytes << 3;
    static const Int ONE = 1;

#define MASK_AND(n) ((ONE << n) - 1)

public:
    BitStream(std::ios &ios, Type type, std::size_t bufferSize = 4 << 20);

    template<typename T> T read(T count);

    // write 'count' LSBs
    BitStream& write(Int value, int count);

    template <typename Iterator>
    BitStream& write(Iterator first, Iterator last);

    template <typename BitSet>
    friend 
    BitStream& operator<< (BitStream& stream, BitSet const &bitset)
    {
        return stream.write(bitset.begin(), bitset.end());
    }

    void flush(bool writeExtraBits = true);

    std::size_t bit_count() const {
        return nbits * _buffer.size() + (nbits - _remainingBits);
    }

    std::size_t remaining_bits() const { return _remainingBits; }

private:
    void seek_buffer() {
        if (++_bufferindex == _buffer.end())
            fill_buffer();
        _bufftop = *_bufferindex;
    }

    void fill_buffer();

#undef MASK_AND
};

extern template BitStream::Int 
BitStream::read<BitStream::Int>(BitStream::Int);

#endif // __BITSTREAM_HH__
