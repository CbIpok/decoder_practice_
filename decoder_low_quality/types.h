#pragma once
#include <cstdint>
#include <vector>



constexpr uint8_t PKT_HDR_DATA_SIZE_SHORT = 15;
constexpr uint8_t PKT_HDR_DATA_SIZE_LONG = 20;
constexpr uint8_t PKT_HDR_GCLI_SIZE_SHORT = 13;
constexpr uint8_t PKT_HDR_GCLI_SIZE_LONG = 20;
constexpr uint8_t PKT_HDR_SIGN_SIZE_SHORT = 11;
constexpr uint8_t PKT_HDR_SIGN_SIZE_LONG = 15;
constexpr uint8_t PKT_HDR_ALIGNMENT = 8;

constexpr uint16_t XS_MARKER_NBYTES = 2;

constexpr size_t BYTE_SIZE = 8;

constexpr uint8_t PRESINCT_PER_SLISE = 8; // todo find out from standart, may be not constant

constexpr uint8_t PACKETS_PER_PRESINCT = 4; // todo find out from standart, may be not constant

enum class eMarker : uint16_t { 
    SOC = 0xff10,
    EOC = 0xff11,
    PIH = 0xff12,
    CDT = 0xff13,
    WGT = 0xff14,
    COM = 0xff15,
    SLH = 0xff20,
    CAP = 0xff50,
    DEFAULT,
};



struct NetworkPacket //todo write in cpp
{
    NetworkPacket(uint8_t* data, size_t len):
        data((new uint8_t[len]())),
        size(len)
    {
        memcpy(this->data.get(), data, len);
    }
    NetworkPacket(const NetworkPacket& packet):
        data((new uint8_t[packet.size]())),
        size(packet.size)
    {
        memcpy(this->data.get(), packet.data.get(), size);
    }
    std::unique_ptr<uint8_t[]> data;
    size_t size;
};

struct PictureHeader
{
    uint32_t  codestreamSize;
    uint16_t profile;
    uint16_t level;
    uint16_t frameWidth;
    uint16_t frameHeight;
    uint16_t precinctWidth;
    uint16_t  slicehHeight;
    uint8_t componentsNumber;
    uint8_t codeGroupSize;
    uint8_t significanceGroupSize;
    uint8_t  waveletBitPrecision;
    uint8_t fractionalBits : 4; //4 bit
    uint8_t rawBitsPerCodeGroup : 4; //4 bit
    uint8_t sliceCodingMode : 1; //1 bit
    uint8_t progressionMode : 3; //3 bit
    uint8_t colourDecorrelation : 4; //4 bit
    uint8_t horizontalWaveletLevels : 4; //4 bit
    uint8_t verticalWaveletLevels : 4; // 4 bit
    uint8_t quantizerType : 4; //4 bit
    uint8_t signHandling : 2; //2 bit
    uint8_t runMode : 2; //2 bit

};


struct Component
{
    uint8_t bitPrecision;
    uint8_t horizontalSampling : 4;
    uint8_t verticalSampling : 4;

};


struct Band
{
    uint8_t gain;
    uint8_t priority;
};

struct PacketHeader
{
    uint8_t rawMode : 1;
    uint32_t SizeOfData;
    uint32_t sizeOfTheBitplaneCountSubpacket;
    uint16_t signSubpacketSize;
};

struct PacketBody
{

};


struct PrecinctHeader
{
    uint32_t precinctSize : 24;
    uint8_t quantization;
    uint8_t refinement;
    std::vector<uint8_t> bandCodingmMode; //2 bit
    uint8_t padding;
};

struct Precinct
{
    PrecinctHeader presinctHeader;
    std::vector<PacketBody> packetsBody;
    std::vector<PacketHeader> packetsHeader;
};

struct Slice
{
    uint16_t id;
    std::vector<Precinct> presincts;
};

template<typename T>
constexpr size_t bitCount(T)
{
    size_t result = 0;

    T value = 0;

    value = ~value;

    while (value)
    {
        value &= (value - 1);
        ++result;
    }

    return result;
}
