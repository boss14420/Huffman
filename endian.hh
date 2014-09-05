/*
 * =====================================================================================
 *
 *       Filename:  endian.hh
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/05/2014 02:34:07 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  BOSS14420 (), firefoxlinux at gmail dot com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef __ENDIAN_HH__
#define __ENDIAN_HH__

#include <cstdint>

//enum hl_endianness : std::uint32_t {
//    HL_LITTLE_ENDIAN   = 0x03020100,
//    HL_BIG_ENDIAN      = 0x00010203,
//    HL_PDP_ENDIAN      = 0x01000302,
//    HL_UNKNOWN_ENDIAN  = 0xFFFFFFFF
//};
enum hl_endianness : uint32_t {
    HL_LITTLE_ENDIAN   = 0x00000001,
    HL_BIG_ENDIAN      = 0x01000000,
    HL_PDP_ENDIAN      = 0x00010000,
    HL_UNKNOWN_ENDIAN  = 0xFFFFFFFF
};
static constexpr std::uint32_t value = 0x00010203;
static constexpr const std::uint8_t* endianValues 
                = reinterpret_cast<std::uint8_t const*>(&value);

static constexpr union {
    std::uint8_t b[4];
    std::uint32_t i;
} __bytes { { 0, 1, 2, 3 } };

constexpr hl_endianness getEndianOrder()
{
//#define VALUE (*(std::uint32_t*)endianValues)
////#define VALUE (__bytes.i)
//    return (VALUE == HL_LITTLE_ENDIAN) ? HL_LITTLE_ENDIAN 
//                : (VALUE == HL_BIG_ENDIAN) ? HL_BIG_ENDIAN
//                        : (VALUE == HL_PDP_ENDIAN) ? HL_PDP_ENDIAN
//                                : HL_UNKNOWN_ENDIAN;
//#undef VALUE
//    return (endianValues[0] == 3) ? HL_LITTLE_ENDIAN
//            : (endianValues[0] == 0) ? HL_BIG_ENDIAN
//                : (endianValues[0] == 1) ? HL_PDP_ENDIAN
//                    : HL_UNKNOWN_ENDIAN;

    return
        ((1 & 0xFFFFFFFF) == HL_LITTLE_ENDIAN)
            ? HL_LITTLE_ENDIAN
            : ((1 & 0xFFFFFFFF) == HL_BIG_ENDIAN)
                ? HL_BIG_ENDIAN
                : ((1 & 0xFFFFFFFF) == HL_PDP_ENDIAN)
                    ? HL_PDP_ENDIAN
                    : HL_UNKNOWN_ENDIAN;
}

//#define HL_ENDIANNESS getEndianOrder()
static constexpr auto HL_ENDIANNESS = getEndianOrder();
//static constexpr auto HL_ENDIANNESS = HL_BIG_ENDIAN;

#endif // __ENDIAN_HH__
