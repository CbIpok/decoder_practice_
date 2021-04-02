#include "bitstream.h"
#include <cstdint>
#include <cstring>
#include <cassert>
#include <bitset>

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

    assert(bitsCount < 32);

    if (bitsCount <= _curPacket.numBits)
    {
        _curPacket.numBits -= bitsCount;
        return (_cash >> _curPacket.numBits) & _make_mask(bitsCount);
    }
    return uint32_t();
}

bool FifoBuf::_load_bits()
{
    if (_buf.empty())
    {
        return false;
    }
    if (_curPacket.buf == nullptr)
    {
        _curPacket = Bitstream(_buf.front().data.get(), _buf.front().size);
    }
    if (_curPacket.size - _curPacket.lenReaded >= sizeof(uint64_t)) //if packet have enough data
    {
        readFromBitsream(_curPacket, (uint8_t*)&_cash, sizeof(uint64_t));
    }
    else
    {
        size_t lenReaded = _curPacket.size - _curPacket.lenReaded;
        readFromBitsream(_curPacket, (uint8_t*)&_cash, _curPacket.size - _curPacket.lenReaded);
        _buf.pop_front();
        _curPacket = Bitstream(_buf.front().data.get(), _buf.front().size);
        assert(_curPacket.size - _curPacket.lenReaded >= sizeof(uint64_t));
        readFromBitsream(_curPacket, (uint8_t*)&_cash + lenReaded, sizeof(uint64_t) - lenReaded);
    }

    _cash = _byteswap_uint64(_cash);
    _cashIsEmpty = false;
    return true;
}

constexpr uint32_t FifoBuf::_make_mask(size_t bitsCount) //todo how make it constexpress? _offset 
{
    std::bitset<32> mask{0};
    for (size_t i = 0; i < bitsCount; i++)
    {
        mask[i] = 1;
    }
    return mask.to_ulong();
}


