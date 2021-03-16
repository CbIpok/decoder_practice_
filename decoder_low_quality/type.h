#pragma once
#include <cstdint>


constexpr uint16_t XS_MARKER_PIH = 0xff12;
constexpr uint16_t XS_MARKER_NBYTES = 2;
constexpr uint16_t XS_MARKER_SOC = 0xff10;
constexpr uint16_t XS_MARKER_CAP = 0xff50;
constexpr uint16_t XS_MARKER_SLH = 0xff20;
constexpr uint16_t XS_MARKER_EOC = 0xff11;

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
    uint8_t fractionalBits; //4 bit
    uint8_t rawBitsPerCodeGroup; //4 bit
    uint8_t sliceCodingMode; //1 bit
    uint8_t progressionMode; //3 bit
    uint8_t colourDecorrelation; //4 bit
    uint8_t horizontalWaveletLevels; //4 bit
    uint8_t verticalWaveletLevels; // 4 bit
    uint8_t quantizerType; //4 bit
    uint8_t signHandling; //2 bit
    uint8_t runMode; //2 bit

};