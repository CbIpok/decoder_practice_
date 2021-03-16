#pragma once
#include <cstdint>
#include <vector>

constexpr uint16_t XS_MARKER_PIH = 0xff12;
constexpr uint16_t XS_MARKER_NBYTES = 2;
constexpr uint16_t XS_MARKER_SOC = 0xff10;
constexpr uint16_t XS_MARKER_CAP = 0xff50;
constexpr uint16_t XS_MARKER_SLH = 0xff20;
constexpr uint16_t XS_MARKER_EOC = 0xff11;
constexpr size_t BYTE_SIZE = 8;

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