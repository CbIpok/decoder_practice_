#pragma once

#include <cstdint>
#include <climits>

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
    uint8_t *cur;
    uint8_t *buf;
    uint8_t bitoffset;
};


void readFromBitsream(Bitstream& bitstream, uint8_t* dst, size_t size);


template <typename T>
void readFromBitsreamAndSwap(Bitstream& bitstream, T& dst, size_t size);


template<typename T>
inline void readFromBitsreamAndSwap(Bitstream& bitstream, T& dst, size_t size)
{
    readFromBitsream(bitstream, (uint8_t*)&dst, size);
    dst = swap_endian(dst);
}

template<typename T>
inline void peekBitsreamAndSwap(Bitstream& bitstream, T& dst, size_t size)
{
    readFromBitsream(bitstream, (uint8_t*)&dst, size);
    bitstream.len_readed -= size;
    bitstream.cur -= size;
    dst = swap_endian(dst);
}

void bistreamSkip(Bitstream& bitstream, size_t skipLen);