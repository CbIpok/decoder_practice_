#include "Parser.h"

#include <cassert>

#include "bitstream.h"
#include <fstream>
#include <iostream>
#include <cmath>
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
	uint16_t marker = 0;
	while (marker != XS_MARKER_EOC)
	{
		slices.push_back(getSlice());
		peekBitsreamAndSwap(bitstream, marker, XS_MARKER_NBYTES);
	}
	return slices;
}


BlockOfMemory BlockParser::getSlice()
{
	uint8_t* data{nullptr};
	Bitstream localBistream = bitstream;
	size_t sliceSize = getSliceSize(localBistream);
	data = new uint8_t[sliceSize];
	readFromBitsream(bitstream, data, sliceSize);
	return BlockOfMemory(data, sliceSize);
}




size_t BlockParser::getSliceSize(Bitstream lbitsream)
{
	size_t size;
	uint16_t marker;
	uint16_t val16;
	uint32_t val32;
	readFromBitsreamAndSwap(lbitsream, marker, XS_MARKER_NBYTES);
	assert(marker == XS_MARKER_SLH);
	readFromBitsreamAndSwap(lbitsream, val16, sizeof(val16));
	assert(val16 == 4); // todo Lslh constant
	readFromBitsreamAndSwap(lbitsream, val16, sizeof(val16)); // slice number read
	
	//read all precincts in slice
	while (val16 != XS_MARKER_SLH && val16 != XS_MARKER_EOC)
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

//BlockOfMemory BlockParser::getSlice()
//{
//	
//	return BlockOfMemory();
//}



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
    data(blockOfMemory.data),
    len(blockOfMemory.len)
{
    blockOfMemory.data = nullptr;
}

BlockOfMemory::~BlockOfMemory()
{
     
}



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
