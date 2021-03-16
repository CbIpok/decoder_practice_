#pragma once

#include <cstdint>
#include <climits>
#include <vector>
#include <algorithm>
#include "types.h"
template <typename T>
T swap_endian(T u)
{
    static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

    union
    {
        T u;
        unsigned char u8[sizeof(T)];
    } source, dest;

    source.u = u;

    for (size_t k = 0; k < sizeof(T); k++)
        dest.u8[k] = source.u8[sizeof(T) - k - 1];

    return dest.u;
}

struct Bitstream
{
    size_t size;
    size_t len_readed;
    uint8_t* cur;
    uint8_t* buf;
    uint8_t bitoffset;
};


void readFromBitsream(Bitstream& bitstream, uint8_t* dst, size_t size);




template<typename T>
T readFromBitsreamAndSwap(Bitstream& bitstream) 
{
    T dst;
    readFromBitsream(bitstream, (uint8_t*)&dst, sizeof(T));
    dst = swap_endian(dst);
    return dst;
}

template<typename T>
T peekBitsreamAndSwap(Bitstream& bitstream) 
{
    T dst;
    readFromBitsream(bitstream, (uint8_t*)&dst, sizeof(T));
    bitstream.len_readed -= sizeof(T);
    bitstream.cur -= sizeof(T);
    dst = swap_endian(dst);
    return dst;
}

template<typename T>
T readBitsFromBitstream(Bitstream& bitstream, size_t bitsCount)
{
    std::vector<size_t> compliteTypeSizes{ 8,16,32 };
    T value{ 0 };
    if (std::find(compliteTypeSizes.begin(), compliteTypeSizes.end(), sizeof(T)) == compliteTypeSizes.end())
    {
        T mask{ 0 };
        mask = ~mask;
        T mask1 = mask >> (bitstream.bitoffset + bitsCount);
        T mask2 = mask << (sizeof(T)*BYTE_SIZE - bitstream.bitoffset);
        mask = ~(mask1 | mask2); // |000000...|(a)111111...|(b)000000...|  a pos is offset, b pos is bitsCount + offset
        value = peekBitsreamAndSwap<T>(bitstream);
        value = value & mask;
        size_t valueShitRight = sizeof(T)*BYTE_SIZE - bitsCount - bitstream.bitoffset;
        value = value >> valueShitRight;
        bitstream.bitoffset += bitsCount;
        while (bitstream.bitoffset >= 8)
        {
            bitstream.len_readed += 1;
            bitstream.cur += 1;
            bitstream.bitoffset -= 8;
        }
    }
    else
    {

        value = readFromBitsreamAndSwap<T>(bitstream);
    }

    return value;
}

void bistreamSkip(Bitstream& bitstream, size_t skipLen);