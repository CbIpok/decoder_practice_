#include "Parser.h"

#include <cassert>
#include "bitstream.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <algorithm>
#include "bitstream.h"

BlockParser::BlockParser(Bitstream&& bitstream) :
    bitstream(std::move(bitstream))
{

}

BlockParser::BlockParser(uint8_t* bitstream, size_t size)
{
    this->bitstream.buf = bitstream;
    this->bitstream.cur = bitstream;
    this->bitstream.len_readed = 0;
    this->bitstream.size = size;

}


BlockOfMemory BlockParser::getHeader()
{
    uint8_t* buf = new uint8_t[HEADER_SIZE];

    readFromBitsream(bitstream, buf, HEADER_SIZE);
    return { buf, HEADER_SIZE };
}

std::vector<BlockOfMemory> BlockParser::getSlices()
{
    std::vector<BlockOfMemory> slices;
    eMarker marker{ eMarker::DEFAULT };
    while (marker != eMarker::EOC)
    {
        slices.push_back(getSlice());
        marker = peekBitsreamAndSwap<eMarker>(bitstream);
    }
    return slices;
}


BlockOfMemory BlockParser::getSlice()
{
    uint8_t* data{ nullptr };
    Bitstream localBistream = bitstream;
    size_t sliceSize = getSliceSize(localBistream);
    data = new uint8_t[sliceSize];
    readFromBitsream(bitstream, data, sliceSize);
    return BlockOfMemory(data, sliceSize);
}




size_t BlockParser::getSliceSize(Bitstream& lbitsream) //todo use eMarker
{
    size_t size;
    eMarker marker;
    uint16_t val16;
    uint32_t val32;
    marker = readFromBitsreamAndSwap<eMarker>(lbitsream);
    assert(marker == eMarker::SLH);
    val16 = readFromBitsreamAndSwap<uint16_t>(lbitsream);
    uint16_t Lslh = 4; // constant from 1st part of standart
    assert(val16 == Lslh);
    val16 = readFromBitsreamAndSwap<uint16_t>(lbitsream); // slice number read

    //read all precincts in slice
    while (val16 != XS_MARKER_SLH && val16 != XS_MARKER_EOC && bitstream.len_readed < bitstream.size)
    {
        val32 = peekBitsreamAndSwap<uint32_t>(lbitsream);
        uint32_t precinctDataSize = val32 >> 8; // read 24 bits
        size_t precinctOverhead = 11;
        size_t precincSize = precinctDataSize + precinctOverhead;
        bistreamSkip(lbitsream, precincSize);
        val16 = peekBitsreamAndSwap<uint16_t>(lbitsream); // possible read Slice header
    }

    size = lbitsream.len_readed - bitstream.len_readed;
    return size;
}




void writeBlockOfMemoryToFile(const BlockOfMemory& blockOfMemory, const std::string& fileName)
{

    try
    {
        std::ofstream out(fileName, std::ios::binary);
        //out.write((char*)blockOfMemory.first, blockOfMemory.second);
        out.write((char*)blockOfMemory.data.get(), blockOfMemory.len);
    }
    catch (const std::exception& e)
    {
        std::cout << "cannot write to file";
    }
}


BlockOfMemory::BlockOfMemory(uint8_t* data, size_t size) :
    data(data),
    len(size)
{
    assert(data != nullptr);
    bitstream.buf = data;
    bitstream.cur = data;
    bitstream.len_readed = 0;
    bitstream.size = size;
}


BlockOfMemory::BlockOfMemory(BlockOfMemory&& blockOfMemory) noexcept :
    data(std::move(blockOfMemory.data)),
    len(blockOfMemory.len),
    bitstream(blockOfMemory.bitstream)
{
    blockOfMemory.data = nullptr;
}

BlockOfMemory::~BlockOfMemory()
{

}



void DetailParser::parseHeader(BlockOfMemory& blockOfMemory)
{
    
    int LenghtPih = 26;
    int precinctHeight;
    eMarker marker;
    uint32_t val32 = 0;
    uint16_t val16 = 0;
    uint8_t val8 = 0;


    //SOC marker 
    {
        marker = readFromBitsreamAndSwap<eMarker>(blockOfMemory.bitstream);
        //marker = readBitsFromBitstream(blockOfMemory.bitstream,)
        assert(marker == eMarker::SOC);
    }
    //CAP markers 
    {
        marker = readFromBitsreamAndSwap<eMarker>(blockOfMemory.bitstream);
        assert(marker == eMarker::CAP);
        //Lcap
        val16 = readFromBitsreamAndSwap<uint16_t>(blockOfMemory.bitstream);
        assert(val16 == 2);
    }
    
    {
        marker = readFromBitsreamAndSwap<eMarker>(blockOfMemory.bitstream);
        assert(marker == eMarker::PIH);
        val16 = readFromBitsreamAndSwap<uint16_t>(blockOfMemory.bitstream);
        assert(val16 == LenghtPih);
        val32 = readFromBitsreamAndSwap<uint32_t>(blockOfMemory.bitstream);
        pictureHeader.codestreamSize = val32;

        val16 = readFromBitsreamAndSwap<uint16_t>(blockOfMemory.bitstream);
        pictureHeader.profile = val16;

        val16 = readFromBitsreamAndSwap<uint16_t>(blockOfMemory.bitstream);
        pictureHeader.level = val16;

        val16 = readFromBitsreamAndSwap<uint16_t>(blockOfMemory.bitstream);
        pictureHeader.frameHeight = val16;

        val16 = readFromBitsreamAndSwap<uint16_t>(blockOfMemory.bitstream);
        pictureHeader.frameWidth = val16;


        val16 = readFromBitsreamAndSwap<uint16_t>(blockOfMemory.bitstream);
        pictureHeader.precinctWidth = val16;

        val16 = readFromBitsreamAndSwap<uint16_t>(blockOfMemory.bitstream);
        pictureHeader.slicehHeight = val16;

        val8 = readFromBitsreamAndSwap<uint8_t>(blockOfMemory.bitstream);
        pictureHeader.componentsNumber = val8;

        val8 = readFromBitsreamAndSwap<uint8_t>(blockOfMemory.bitstream);
        pictureHeader.codeGroupSize = val8;

        val8 = readFromBitsreamAndSwap<uint8_t>(blockOfMemory.bitstream);
        pictureHeader.significanceGroupSize = val8;

        val8 = readFromBitsreamAndSwap<uint8_t>(blockOfMemory.bitstream);
        pictureHeader.waveletBitPrecision = val8;


        pictureHeader.fractionalBits = readBitsFromBitstream<uint8_t>(blockOfMemory.bitstream, 4);

        pictureHeader.rawBitsPerCodeGroup = readBitsFromBitstream<uint8_t>(blockOfMemory.bitstream, 4);
        assert(pictureHeader.rawBitsPerCodeGroup == 4);


        pictureHeader.sliceCodingMode = readBitsFromBitstream<uint8_t>(blockOfMemory.bitstream, 1);

        pictureHeader.progressionMode = readBitsFromBitstream<uint8_t>(blockOfMemory.bitstream, 3); 

        pictureHeader.colourDecorrelation = readBitsFromBitstream<uint8_t>(blockOfMemory.bitstream, 4);

        pictureHeader.horizontalWaveletLevels = readBitsFromBitstream<uint8_t>(blockOfMemory.bitstream, 4);

        pictureHeader.verticalWaveletLevels = readBitsFromBitstream<uint8_t>(blockOfMemory.bitstream, 4);
        precinctHeight = (1 << pictureHeader.verticalWaveletLevels);
        pictureHeader.slicehHeight *= precinctHeight;

        pictureHeader.quantizerType = readBitsFromBitstream<uint8_t>(blockOfMemory.bitstream, 4);

        pictureHeader.signHandling = readBitsFromBitstream<uint8_t>(blockOfMemory.bitstream, 2);

        pictureHeader.runMode = readBitsFromBitstream<uint8_t>(blockOfMemory.bitstream, 2);

    }

    
    //parse CDT
    {
        marker = readFromBitsreamAndSwap<eMarker>(blockOfMemory.bitstream);
        assert((eMarker)marker == eMarker::CDT);
        
        val16 = readFromBitsreamAndSwap<uint16_t>(blockOfMemory.bitstream);// Lcdt Size of the segment in bytes, not including
        
        for (size_t i = 0; i < pictureHeader.componentsNumber; i++)
        {
            componentTable.emplace_back();

            val8 = readFromBitsreamAndSwap<uint8_t>(blockOfMemory.bitstream);
            componentTable[i].bitPrecision = val8;

            val8 = readBitsFromBitstream<uint8_t>(blockOfMemory.bitstream, 4);
            componentTable[i].horizontalSampling = val8;

            val8 = readBitsFromBitstream<uint8_t>(blockOfMemory.bitstream, 4);
            componentTable[i].verticalSampling = val8;
        }

        
    }
    //parse WGT
    
    {
        marker = readFromBitsreamAndSwap<eMarker>(blockOfMemory.bitstream);
        assert((eMarker)marker == eMarker::WGT);

        val16 = readFromBitsreamAndSwap<uint16_t>(blockOfMemory.bitstream);// Size of the segment in bytes, not including the marker

        size_t bandPerComponent = 2 * std::min({ pictureHeader.horizontalWaveletLevels, pictureHeader.verticalWaveletLevels }) + std::max({ pictureHeader.horizontalWaveletLevels, pictureHeader.verticalWaveletLevels }) + 1;

        weightTable.resize(bandPerComponent* pictureHeader.componentsNumber);
        for (size_t band = 0; band < bandPerComponent; band++)
        {
            for (size_t component = 0; component < pictureHeader.componentsNumber; component++)
            {
                
                size_t i = bandPerComponent * component + band;
                val8 = readFromBitsreamAndSwap<uint8_t>(blockOfMemory.bitstream);
                weightTable[i].gain = val8;
                val8 = readFromBitsreamAndSwap<uint8_t>(blockOfMemory.bitstream);
                weightTable[i].priority = val8;
            }
        }
    }




    //return pictureHeader;
}
