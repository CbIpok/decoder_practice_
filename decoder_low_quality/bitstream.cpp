#include "bitstream.h"
#include <cstdint>
#include <cstring>
#include <cassert>
#include <bitset>
#include <array>

constexpr std::array<size_t, sizeof(size_t) * 8 + 1> make_mask() //todo how make it constexpress?
{
    std::array<size_t, sizeof(size_t) * 8 + 1> mask = {};

    size_t n = 0;
    for (auto& item : mask)
    {
        item = n;
        n = (n << 1) | 1;
    }

    return mask;
}

constexpr auto mask = make_mask();

void readFromBitsream(Bitstream& bitstream, uint8_t *dst, size_t size) //todo make method
{
    std::memcpy(dst, bitstream.cur, size);
    bitstream.cur += size;
    bitstream.lenReaded += size;
}

void bistreamSkip(Bitstream& bitstream, size_t skipLen) //make method tode
{
    bitstream.cur += skipLen;
    bitstream.lenReaded += skipLen;
}


Bitstream::Bitstream(uint8_t* data, size_t size):
    cur(data),
    buf(data),
    lenReaded(0),
    size(size),
    bitoffset(0)
{

}

Bitstream::Bitstream(const Bitstream& bitstream):
size(bitstream.size),
lenReaded(bitstream.lenReaded),
cur(bitstream.cur),
buf(bitstream.buf),
bitoffset(bitstream.bitoffset)
{

}

Bitstream::Bitstream() // todo remove
{

}


void FifoBuf::recive(const NetworkPacket& packet)
{
    _buf.emplace_back(packet);
}

uint32_t FifoBuf::readBits(size_t bitsCount) 
{
    //if (_cashIsEmpty)
    //{
    //    _load_cash(); //todo bad case
    //}
    //if (bitsCount <= sizeof(uint64_t)*BYTE_SIZE - _offset)
    //{
    //   /* uint64_t mask{ 0 };
    //    mask = ~mask;
    //    uint64_t mask1 = mask >> (_offset + bitsCount);*/
    //    //uint64_t mask2 = mask << (sizeof(uint64_t) * BYTE_SIZE - _offset);
    //    /*uint64_t mask2 = mask << 64;
    //    uint64_t mask3 = mask << 63;*/
    //    /*mask = ~(mask1 | mask2);*/ // |000000...|(a)111111...|(b)000000...|  a pos is offset, b pos is bitsCount + offset
    //    uint64_t mask = _make_mask(bitsCount);
    //    uint64_t value = (_cash & mask);
    //    size_t valueShitRight = sizeof(uint64_t) * BYTE_SIZE - bitsCount - _offset;
    //    value = value >> valueShitRight;
    //    _offset += bitsCount;
    //    if (_offset == sizeof(uint64_t))
    //    {
    //        _cashIsEmpty = true;
    //    }
    //    return value;
    //}
    //else
    //{
    //    assert(false);
    //}
    //return (uint64_t)nullptr;

    assert(bitsCount <= 32); //todo read more than 32 bit

    size_t value;
    if (bitsCount <= _curPacket.numBits)
    {
        _curPacket.numBits -= bitsCount;
        value = (_cash >> _curPacket.numBits) & mask[bitsCount];
    }
    else
    {
        value = 0; 
        readBits(_curPacket.numBits);


        uint32_t nextBits = _load_bits();
        _curPacket.numBits += 32 - bitsCount; //todo magic number
        _cash = (_cash << 32) | nextBits;
        return (_cash >> _curPacket.numBits) & mask[bitsCount];
    }

    return value;
}

uint32_t FifoBuf::_load_bits()
{
    uint32_t nextBits;
    if (_buf.empty())
    {
        return false;
    }
    if (_curPacket.buf == nullptr)
    {
        _curPacket = Bitstream(_buf.front().data.get(), _buf.front().size);
    }
    if (_curPacket.size - _curPacket.lenReaded >= sizeof(uint32_t)) //if packet have enough data
    {
        nextBits = (_curPacket.cur[0] << 24) |
                            (_curPacket.cur[1] << 16) |
                            (_curPacket.cur[2] << 8) |
                            (_curPacket.cur[3]);
        bistreamSkip(_curPacket, 4);

    }
    else
    {
        /*size_t lenReaded = _curPacket.size - _curPacket.lenReaded;
        readFromBitsream(_curPacket, (uint8_t*)&_cash, _curPacket.size - _curPacket.lenReaded);
        _buf.pop_front();
        _curPacket = Bitstream(_buf.front().data.get(), _buf.front().size);
        assert(_curPacket.size - _curPacket.lenReaded >= sizeof(uint64_t));
        readFromBitsream(_curPacket, (uint8_t*)&_cash, _curPacket.size - _curPacket.lenReaded);
        bistreamSkip(_curPacket, 4);*/
        assert(false);
    }

    //nextBits = _byteswap_ulong(nextBits);
    return nextBits;
    
}
