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

BlockParser::BlockParser(uint8_t* rawStream, size_t size)
	: bitstream(rawStream, size)
{

}

BlockOfMemory BlockParser::getHeader()
{
    auto* buf = new uint8_t[HEADER_SIZE];

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
        peekBitsreamAndSwap(bitstream, marker, XS_MARKER_NBYTES);
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

<<<<<<< HEAD



size_t BlockParser::getSliceSize(Bitstream& lbitsream) //todo use eMarker
=======
size_t BlockParser::getSliceSize(Bitstream& lbitsream)
>>>>>>> 6835424e78c01eed3717087e323f3c69f89f04fd
{
    size_t size;
    eMarker marker;
    uint16_t val16;
    uint32_t val32;
    readFromBitsreamAndSwap(lbitsream, marker, XS_MARKER_NBYTES);
    assert(marker == eMarker::SLH);
    readFromBitsreamAndSwap(lbitsream, val16, sizeof(val16));
    uint16_t Lslh = 4; // constant from 1st part of standart
    assert(val16 == Lslh);
    readFromBitsreamAndSwap(lbitsream, val16, sizeof(val16)); // slice number read

    //read all precincts in slice
    while (val16 != XS_MARKER_SLH && val16 != XS_MARKER_EOC && bitstream.len_readed < bitstream.size)
    {
        peekBitsreamAndSwap(lbitsream, val32, sizeof(val32));
        uint32_t precinctDataSize = val32 >> 8; // read 24 bits
        size_t precinctOverhead = 11;
        size_t precincSize = precinctDataSize + precinctOverhead;
        bistreamSkip(lbitsream, precincSize);
        peekBitsreamAndSwap(lbitsream, val16, sizeof(val16)); // possible read Slice header
    }

    size = lbitsream.len_readed - bitstream.len_readed;
    return size;
}

void writeBlockOfMemoryToFile(const BlockOfMemory& blockOfMemory, const std::string& fileName)
{

    try
    {
        std::ofstream out(fileName, std::ios::binary);
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

<<<<<<< HEAD
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
        readFromBitsreamAndSwap(blockOfMemory.bitstream, marker, XS_MARKER_NBYTES);
        //marker = readBitsFromBitstream(blockOfMemory.bitstream,)
        assert(marker == eMarker::SOC);
    }
    //CAP markers 
    {
        readFromBitsreamAndSwap(blockOfMemory.bitstream, marker, XS_MARKER_NBYTES);
        assert(marker == eMarker::CAP);
        //Lcap
        readFromBitsreamAndSwap(blockOfMemory.bitstream, val16, sizeof(val16));
        assert(val16 == 2);
    }
    
    {
        readFromBitsreamAndSwap(blockOfMemory.bitstream, marker, XS_MARKER_NBYTES);
        assert(marker == eMarker::PIH);
        readFromBitsreamAndSwap(blockOfMemory.bitstream, val16, sizeof(val16));
        assert(val16 == LenghtPih);
        readFromBitsreamAndSwap(blockOfMemory.bitstream, val32, sizeof(val32));
        pictureHeader.codestreamSize = val32;

        readFromBitsreamAndSwap(blockOfMemory.bitstream, val16, sizeof(val16));
        pictureHeader.profile = val16;

        readFromBitsreamAndSwap(blockOfMemory.bitstream, val16, sizeof(val16));
        pictureHeader.level = val16;

        readFromBitsreamAndSwap(blockOfMemory.bitstream, val16, sizeof(val16));
        pictureHeader.frameHeight = val16;

        readFromBitsreamAndSwap(blockOfMemory.bitstream, val16, sizeof(val16));
        pictureHeader.frameWidth = val16;


        readFromBitsreamAndSwap(blockOfMemory.bitstream, val16, sizeof(val16));
        pictureHeader.precinctWidth = val16;

        readFromBitsreamAndSwap(blockOfMemory.bitstream, val16, sizeof(val16));
        pictureHeader.slicehHeight = val16;

        readFromBitsreamAndSwap(blockOfMemory.bitstream, val8, sizeof(val8));
        pictureHeader.componentsNumber = val8;

        readFromBitsreamAndSwap(blockOfMemory.bitstream, val8, sizeof(val8));
        pictureHeader.codeGroupSize = val8;

        readFromBitsreamAndSwap(blockOfMemory.bitstream, val8, sizeof(val8));
        pictureHeader.significanceGroupSize = val8;

        readFromBitsreamAndSwap(blockOfMemory.bitstream, val8, sizeof(val8));
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
        readFromBitsreamAndSwap(blockOfMemory.bitstream, marker, sizeof(marker));
        assert((eMarker)marker == eMarker::CDT);
        
        readFromBitsreamAndSwap(blockOfMemory.bitstream, val16, sizeof(marker));// Lcdt Size of the segment in bytes, not including
        
        for (size_t i = 0; i < pictureHeader.componentsNumber; i++)
        {
            componentTable.emplace_back();

            readFromBitsreamAndSwap(blockOfMemory.bitstream, val8, sizeof(val8));
            componentTable[i].bitPrecision = val8;

            val8 = readBitsFromBitstream<uint8_t>(blockOfMemory.bitstream, 4);
            componentTable[i].horizontalSampling = val8;

            val8 = readBitsFromBitstream<uint8_t>(blockOfMemory.bitstream, 4);
            componentTable[i].verticalSampling = val8;
        }

        
    }
    //parse WGT
    
    {
        readFromBitsreamAndSwap(blockOfMemory.bitstream, marker, sizeof(marker));
        assert((eMarker)marker == eMarker::WGT);

        readFromBitsreamAndSwap(blockOfMemory.bitstream, val16, sizeof(val16));// Size of the segment in bytes, not including the marker

        size_t bandPerComponent = 2 * std::min({ pictureHeader.horizontalWaveletLevels, pictureHeader.verticalWaveletLevels }) + std::max({ pictureHeader.horizontalWaveletLevels, pictureHeader.verticalWaveletLevels }) + 1;

        weightTable.resize(bandPerComponent* pictureHeader.componentsNumber);
        for (size_t band = 0; band < bandPerComponent; band++)
        {
            for (size_t component = 0; component < pictureHeader.componentsNumber; component++)
            {
                
                size_t i = bandPerComponent * component + band;
                readFromBitsreamAndSwap(blockOfMemory.bitstream, val8, sizeof(val8));
                weightTable[i].gain = val8;
                readFromBitsreamAndSwap(blockOfMemory.bitstream, val8, sizeof(val8));
                weightTable[i].priority = val8;
            }
        }
    }




    //return pictureHeader;
}
=======
PictureHeader DetailParser::parseHeader(BlockOfMemory& blockOfMemory)
{
	PictureHeader pictureHeader;
	int LenghtPih = 26; 
	int precinctHeight; 
	uint16_t marker;
	uint32_t val32 = 0;
	uint16_t val16 = 0;
	uint8_t val8 = 0;

	
	//SOC marker 
	{
		readFromBitsreamAndSwap(blockOfMemory.bitstream, marker, XS_MARKER_NBYTES);
		assert(marker == XS_MARKER_SOC);
	}
	//CAP markers
	{
		readFromBitsreamAndSwap(blockOfMemory.bitstream, marker, XS_MARKER_NBYTES);
		assert(marker == XS_MARKER_CAP);
		//Lcap
		readFromBitsreamAndSwap(blockOfMemory.bitstream, marker, XS_MARKER_NBYTES);
		assert(marker == 2); 
	}
	//PIH markers and data
	{
		readFromBitsreamAndSwap(blockOfMemory.bitstream, marker, XS_MARKER_NBYTES);
		assert(marker == XS_MARKER_PIH);
		readFromBitsreamAndSwap(blockOfMemory.bitstream, val16, sizeof(val16));
		assert(val16 == LenghtPih);
		readFromBitsreamAndSwap(blockOfMemory.bitstream, val32, sizeof(val32));
		pictureHeader.codestreamSize = val32;

		readFromBitsreamAndSwap(blockOfMemory.bitstream, val16, sizeof(val16));
		pictureHeader.profile = val16;

		readFromBitsreamAndSwap(blockOfMemory.bitstream, val16, sizeof(val16));
		pictureHeader.level = val16;

		readFromBitsreamAndSwap(blockOfMemory.bitstream, val16, sizeof(val16));
		pictureHeader.frameHeight = val16;

		readFromBitsreamAndSwap(blockOfMemory.bitstream, val16, sizeof(val16));
		pictureHeader.frameWidth = val16;


		readFromBitsreamAndSwap(blockOfMemory.bitstream, val16, sizeof(val16));
		pictureHeader.precinctWidth = val16;

		readFromBitsreamAndSwap(blockOfMemory.bitstream, val16, sizeof(val16));
		pictureHeader.slicehHeight = val16;
		 
		readFromBitsreamAndSwap(blockOfMemory.bitstream, val8, sizeof(val8));
		pictureHeader.componentsNumber = val8;
		 
		readFromBitsreamAndSwap(blockOfMemory.bitstream, val8, sizeof(val8));
		pictureHeader.codeGroupSize = val8;
		 
		readFromBitsreamAndSwap(blockOfMemory.bitstream, val8, sizeof(val8));
		pictureHeader.significanceGroupSize = val8;
	
		readFromBitsreamAndSwap(blockOfMemory.bitstream, val8, sizeof(val8));
		pictureHeader.waveletBitPrecision = val8;
		 
		readFromBitsreamAndSwap(blockOfMemory.bitstream, val8, sizeof(val8));

		pictureHeader.fractionalBits =  val8 >> 4; //read 4

		pictureHeader.rawBitsPerCodeGroup = (val8 & 0b0001111); //read 4
		assert(pictureHeader.rawBitsPerCodeGroup == 4);

		readFromBitsreamAndSwap(blockOfMemory.bitstream, val8, sizeof(val8));

		pictureHeader.sliceCodingMode = val8 >> 7; //read 1
	
		pictureHeader.progressionMode = (val8 >> 4) & 0b00000111; //read 3

		pictureHeader.colourDecorrelation = val8 & 0b0001111; //read 4
		
		readFromBitsreamAndSwap(blockOfMemory.bitstream, val8, sizeof(val8));
		pictureHeader.horizontalWaveletLevels = val8 >> 4; //read 4

		pictureHeader.verticalWaveletLevels = val8 & 0b0001111; //read 4
		precinctHeight = (1 << pictureHeader.verticalWaveletLevels);
		pictureHeader.slicehHeight *= precinctHeight;

		readFromBitsreamAndSwap(blockOfMemory.bitstream, val8, sizeof(val8));

		pictureHeader.quantizerType = val8 >> 4;//read 4

		pictureHeader.signHandling = (val8 >> 2) & (0b00000011); //read 2

		pictureHeader.runMode = val8 & 0b000000011; //read 2

	}
	//parse CDT
	{
		
	}
	//parse WGT
	{

	}
	
	return pictureHeader;
}
>>>>>>> 6835424e78c01eed3717087e323f3c69f89f04fd
