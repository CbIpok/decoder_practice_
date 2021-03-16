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

    for (size_t k = 0; k < sizeof(T); ++k)
        dest.u8[k] = source.u8[sizeof(T) - k - 1];

    return dest.u;
}

struct Bitstream final
{
<<<<<<< HEAD
    size_t size;
    size_t len_readed;
    uint8_t* cur;
    uint8_t* buf;
    uint8_t bitoffset;
};

=======
    Bitstream(uint8_t* bitstream = nullptr, size_t rhsSize = 0):
        cur(bitstream),
        buf(bitstream),
        size(rhsSize),
        len_readed(0)
    {
>>>>>>> 6835424e78c01eed3717087e323f3c69f89f04fd

    }

    uint8_t* cur{ nullptr};
    uint8_t* buf{ nullptr };

    size_t size{ 0 };
    size_t len_readed{ 0 };
};

void readFromBitsream(Bitstream& bitstream, uint8_t* dst, size_t size);

template<typename T>
<<<<<<< HEAD
void readFromBitsreamAndSwap(Bitstream& bitstream, T& dst, size_t size) //todo return T, remove size
=======
void readFromBitsreamAndSwap(Bitstream& bitstream, T& dst, size_t size)
>>>>>>> 6835424e78c01eed3717087e323f3c69f89f04fd
{
    readFromBitsream(bitstream, (uint8_t*)&dst, size);
    dst = swap_endian(dst);
}

template<typename T>
<<<<<<< HEAD
void peekBitsreamAndSwap(Bitstream& bitstream, T& dst, size_t size) // todo return T
=======
void peekBitsreamAndSwap(Bitstream& bitstream, T& dst, size_t size)
>>>>>>> 6835424e78c01eed3717087e323f3c69f89f04fd
{
    readFromBitsream(bitstream, (uint8_t*)&dst, size);
    bitstream.len_readed -= size;
    bitstream.cur -= size;
    dst = swap_endian(dst);
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
        peekBitsreamAndSwap(bitstream, value, sizeof(T));
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

        readFromBitsreamAndSwap(bitstream, value, sizeof(T));
    }

    return value;
}

void bistreamSkip(Bitstream& bitstream, size_t skipLen);