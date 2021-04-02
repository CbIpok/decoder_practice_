#pragma once

#include <cstdint>
#include <memory>
#include <climits>
#include <vector>
#include <algorithm>
#include "types.h"
#include <deque>


class Bitstream
{
public:
    Bitstream(uint8_t* data, size_t size);

    Bitstream(const Bitstream& bitstream);

    Bitstream();

    size_t size{ 0 };
    size_t lenReaded{ 0 };
    uint8_t* cur{ nullptr };
    uint8_t* buf{ nullptr };
    size_t numBits {0};
    uint8_t bitoffset{ 0 }; //remove

private:



};

class FifoBuf final
{
public:
    void recive(const NetworkPacket& packet);  

    uint32_t readBits(size_t bitsCount); //todo template

private:
    uint64_t _cash;
    size_t _offset;
    std::deque<NetworkPacket> _buf;
    bool _cashIsEmpty{ true };
    Bitstream _curPacket;

    bool _load_bits();
    constexpr uint32_t _make_mask (size_t bitsCount); 
};







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

//struct Bitstream
//{
//    size_t size{0};
//    size_t len_readed{0};
//    uint8_t* cur{nullptr};
//    uint8_t* buf{nullptr};
//    uint8_t bitoffset{0};
//};


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
    bitstream.lenReaded -= sizeof(T);
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
            bitstream.lenReaded += 1;
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