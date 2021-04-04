#include "Parser.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <cmath>
#include <algorithm>

BlockParser::BlockParser(Bitstream &&bitstream) :
        bitstream(std::move(bitstream)) {

}

BlockParser::BlockParser(uint8_t *bitstream, size_t size) {
    this->bitstream.buf = bitstream;
    this->bitstream.cur = bitstream;
    this->bitstream.lenReaded = 0;
    this->bitstream.size = size;

}


BlockOfMemory BlockParser::getHeader() {
    uint8_t *buf = new uint8_t[HEADER_SIZE];

    readFromBitsream(bitstream, buf, HEADER_SIZE);
    return {buf, HEADER_SIZE};
}

std::vector<BlockOfMemory> BlockParser::getSlices() {
    std::vector<BlockOfMemory> slices;
    eMarker marker{eMarker::DEFAULT};
    while (marker != eMarker::EOC) {
        slices.push_back(getSlice());
        marker = peekBitsreamAndSwap<eMarker>(bitstream);
    }
    return slices;
}


BlockOfMemory BlockParser::getSlice() {
    uint8_t *data{nullptr};
    Bitstream localBistream = bitstream;
    size_t sliceSize = getSliceSize(localBistream);
    data = new uint8_t[sliceSize];
    readFromBitsream(bitstream, data, sliceSize);
    return BlockOfMemory(data, sliceSize);
}


size_t BlockParser::getSliceSize(Bitstream &lbitsream) {
    size_t size;
    eMarker marker;
    uint16_t val16;
    uint32_t val32;
    marker = readFromBitsreamAndSwap<eMarker>(lbitsream);
    assert(marker == eMarker::SLH);
    val16 = readFromBitsreamAndSwap<uint16_t>(lbitsream);
    uint16_t Lslh = 4; // constant from 1st part of standart
    assert(val16 == Lslh);
    marker = readFromBitsreamAndSwap<eMarker>(lbitsream); // slice number read

    //read all precincts in slice
    while (marker != eMarker::SLH && marker != eMarker::EOC && bitstream.lenReaded < bitstream.size) {
        val32 = peekBitsreamAndSwap<uint32_t>(lbitsream);
        uint32_t precinctDataSize = val32 >> 8; // read 24 bits
        size_t precinctOverhead = 11;
        size_t precincSize = precinctDataSize + precinctOverhead;
        bistreamSkip(lbitsream, precincSize);
        marker = peekBitsreamAndSwap<eMarker>(lbitsream); // possible read Slice header
    }

    size = lbitsream.lenReaded - bitstream.lenReaded;
    return size;
}


void writeBlockOfMemoryToFile(const BlockOfMemory &blockOfMemory, const std::string &fileName) {

    try {
        std::ofstream out(fileName, std::ios::binary);
        //out.write((char*)blockOfMemory.first, blockOfMemory.second);
        out.write((char *) blockOfMemory.data.get(), blockOfMemory.len);
    }
    catch (const std::exception &e) {
        std::cout << "cannot write to file";
    }
}


BlockOfMemory::BlockOfMemory(uint8_t *data, size_t size) :
        data(data),
        len(size) {
    assert(data != nullptr);
    bitstream.buf = data;
    bitstream.cur = data;
    bitstream.lenReaded = 0;
    bitstream.size = size;
}


BlockOfMemory::BlockOfMemory(BlockOfMemory &&blockOfMemory) noexcept:
        data(std::move(blockOfMemory.data)),
        len(blockOfMemory.len),
        bitstream(blockOfMemory.bitstream) {
    blockOfMemory.data = nullptr;
}

BlockOfMemory::~BlockOfMemory() {

}


void DetailParser::parseHeader(BlockOfMemory &blockOfMemory) {

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
        assert((eMarker) marker == eMarker::CDT);

        val16 = readFromBitsreamAndSwap<uint16_t>(
                blockOfMemory.bitstream);// Lcdt Size of the segment in bytes, not including

        for (size_t i = 0; i < pictureHeader.componentsNumber; i++) {
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
        assert((eMarker) marker == eMarker::WGT);

        val16 = readFromBitsreamAndSwap<uint16_t>(
                blockOfMemory.bitstream);// Size of the segment in bytes, not including the marker

        size_t bandPerComponent =
                2 * std::min({pictureHeader.horizontalWaveletLevels, pictureHeader.verticalWaveletLevels}) +
                std::max({pictureHeader.horizontalWaveletLevels, pictureHeader.verticalWaveletLevels}) + 1;

        weightTable.resize(bandPerComponent * pictureHeader.componentsNumber);
        for (size_t band = 0; band < bandPerComponent; band++) {
            for (size_t component = 0; component < pictureHeader.componentsNumber; component++) {

                size_t i = bandPerComponent * component + band;
                val8 = readFromBitsreamAndSwap<uint8_t>(blockOfMemory.bitstream);
                weightTable[i].gain = val8;
                val8 = readFromBitsreamAndSwap<uint8_t>(blockOfMemory.bitstream);
                weightTable[i].priority = val8;
            }
        }
    }

}

void DetailParser::parseHeaderFast(FifoBuf &fifo) {
    int LenghtPih = 26;
    int precinctHeight;
    eMarker marker;
    uint32_t val32 = 0;
    uint16_t val16 = 0;
    uint8_t val8 = 0;


    //SOC marker 
    {
        marker = (eMarker) fifo.readBits(sizeof(eMarker) * BYTE_SIZE);
        //marker = readBitsFromBitstream(blockOfMemory.bitstream,)
        assert(marker == eMarker::SOC);
    }
    ////CAP markers 
    //{
    marker = (eMarker) fifo.readBits(sizeof(eMarker) * BYTE_SIZE);
    assert(marker == eMarker::CAP);
    //    //Lcap
    val16 = (uint16_t) fifo.readBits(sizeof(uint16_t) * BYTE_SIZE);
    assert(val16 == 2);
    //}

    //{
    marker = (eMarker) fifo.readBits(sizeof(eMarker) * BYTE_SIZE);
    assert(marker == eMarker::PIH);
    val16 = fifo.readBits(sizeof(uint16_t) * BYTE_SIZE);
    assert(val16 == LenghtPih);
    val32 = (uint32_t) fifo.readBits(sizeof(uint32_t) * BYTE_SIZE);
    pictureHeader.codestreamSize = val32;

    val16 = fifo.readBits(sizeof(uint16_t) * BYTE_SIZE);
    pictureHeader.profile = val16;

    val16 = fifo.readBits(sizeof(uint16_t) * BYTE_SIZE);
    pictureHeader.level = val16;

    val16 = fifo.readBits(sizeof(uint16_t) * BYTE_SIZE);;
    pictureHeader.frameHeight = val16;

    val16 = fifo.readBits(sizeof(uint16_t) * BYTE_SIZE);;
    pictureHeader.frameWidth = val16;


    val16 = fifo.readBits(sizeof(uint16_t) * BYTE_SIZE);
    pictureHeader.precinctWidth = val16;

    val16 = fifo.readBits(sizeof(uint16_t) * BYTE_SIZE);;
    pictureHeader.slicehHeight = val16;

    val8 = fifo.readBits(sizeof(uint8_t) * BYTE_SIZE);
    pictureHeader.componentsNumber = val8;

    val8 = fifo.readBits(sizeof(uint8_t) * BYTE_SIZE);
    pictureHeader.codeGroupSize = val8;

    val8 = fifo.readBits(sizeof(uint8_t) * BYTE_SIZE);
    pictureHeader.significanceGroupSize = val8;

    val8 = fifo.readBits(sizeof(uint8_t) * BYTE_SIZE);
    pictureHeader.waveletBitPrecision = val8;


    pictureHeader.fractionalBits = fifo.readBits(4);

    pictureHeader.rawBitsPerCodeGroup = fifo.readBits(4);
    assert(pictureHeader.rawBitsPerCodeGroup == 4);


    //    pictureHeader.sliceCodingMode = readBitsFromBitstream<uint8_t>(blockOfMemory.bitstream, 1);

    //    pictureHeader.progressionMode = readBitsFromBitstream<uint8_t>(blockOfMemory.bitstream, 3);

    //    pictureHeader.colourDecorrelation = readBitsFromBitstream<uint8_t>(blockOfMemory.bitstream, 4);

    //    pictureHeader.horizontalWaveletLevels = readBitsFromBitstream<uint8_t>(blockOfMemory.bitstream, 4);

    //    pictureHeader.verticalWaveletLevels = readBitsFromBitstream<uint8_t>(blockOfMemory.bitstream, 4);
    //    precinctHeight = (1 << pictureHeader.verticalWaveletLevels);
    //    pictureHeader.slicehHeight *= precinctHeight;

    //    pictureHeader.quantizerType = readBitsFromBitstream<uint8_t>(blockOfMemory.bitstream, 4);

    //    pictureHeader.signHandling = readBitsFromBitstream<uint8_t>(blockOfMemory.bitstream, 2);

    //    pictureHeader.runMode = readBitsFromBitstream<uint8_t>(blockOfMemory.bitstream, 2);

    //}


    ////parse CDT
    //{
    //    marker = readFromBitsreamAndSwap<eMarker>(blockOfMemory.bitstream);
    //    assert((eMarker)marker == eMarker::CDT);

    //    val16 = readFromBitsreamAndSwap<uint16_t>(blockOfMemory.bitstream);// Lcdt Size of the segment in bytes, not including

    //    for (size_t i = 0; i < pictureHeader.componentsNumber; i++)
    //    {
    //        componentTable.emplace_back();

    //        val8 = readFromBitsreamAndSwap<uint8_t>(blockOfMemory.bitstream);
    //        componentTable[i].bitPrecision = val8;

    //        val8 = readBitsFromBitstream<uint8_t>(blockOfMemory.bitstream, 4);
    //        componentTable[i].horizontalSampling = val8;

    //        val8 = readBitsFromBitstream<uint8_t>(blockOfMemory.bitstream, 4);
    //        componentTable[i].verticalSampling = val8;
    //    }


    //}
    ////parse WGT

    //{
    //    marker = readFromBitsreamAndSwap<eMarker>(blockOfMemory.bitstream);
    //    assert((eMarker)marker == eMarker::WGT);

    //    val16 = readFromBitsreamAndSwap<uint16_t>(blockOfMemory.bitstream);// Size of the segment in bytes, not including the marker

    //    size_t bandPerComponent = 2 * std::min({ pictureHeader.horizontalWaveletLevels, pictureHeader.verticalWaveletLevels }) + std::max({ pictureHeader.horizontalWaveletLevels, pictureHeader.verticalWaveletLevels }) + 1;

    //    weightTable.resize(bandPerComponent * pictureHeader.componentsNumber);
    //    for (size_t band = 0; band < bandPerComponent; band++)
    //    {
    //        for (size_t component = 0; component < pictureHeader.componentsNumber; component++)
    //        {

    //            size_t i = bandPerComponent * component + band;
    //            val8 = readFromBitsreamAndSwap<uint8_t>(blockOfMemory.bitstream);
    //            weightTable[i].gain = val8;
    //            val8 = readFromBitsreamAndSwap<uint8_t>(blockOfMemory.bitstream);
    //            weightTable[i].priority = val8;
    //        }
    //    }
    //}
}

//todo refactor
#define MAX_NCOMPS 4
#define MAX_NDECOMP_H 5
#define MAX_NDECOMP_V 2
#define MAX_LVLS_PER_COMP ((MAX_NDECOMP_V+1) * (MAX_NDECOMP_H+1))
#define MAX_LVLS (MAX_LVLS_PER_COMP * MAX_NCOMPS)

void DetailParser::parseSlice(BlockOfMemory &blockOfMemory) {
    //todo refactor
    int gcli_sb_methods[MAX_LVLS];

    Slice slice;
    //read slise header
    {
        size_t size;
        eMarker marker;
        uint16_t val16;
        uint32_t val32;
        marker = readFromBitsreamAndSwap<eMarker>(blockOfMemory.bitstream);
        assert(marker == eMarker::SLH);
        val16 = readFromBitsreamAndSwap<uint16_t>(blockOfMemory.bitstream);
        uint16_t Lslh = 4; // constant from 1st part of standart
        assert(val16 == Lslh);
        uint16_t sliseId = readFromBitsreamAndSwap<uint16_t>(blockOfMemory.bitstream);
    }
    std::vector<Precinct> precincts;
    precincts.resize(PRESINCT_PER_SLISE);
    //read prepresincts
    {
        for (auto &precinct : precincts) {
            //read presinct header
            {
                PrecinctHeader &precinctHeader = precinct.presinctHeader;
                precinctHeader.precinctSize = readBitsFromBitstream<uint32_t>(blockOfMemory.bitstream, 24);
                precinctHeader.quantization = readFromBitsreamAndSwap<uint8_t>(blockOfMemory.bitstream);
                precinctHeader.refinement = readFromBitsreamAndSwap<uint8_t>(blockOfMemory.bitstream);
                size_t bandPerComponent =
                        2 * std::min({pictureHeader.horizontalWaveletLevels, pictureHeader.verticalWaveletLevels}) +
                        std::max({pictureHeader.horizontalWaveletLevels, pictureHeader.verticalWaveletLevels}) + 1;
                precinctHeader.bandCodingmMode.resize(bandPerComponent * pictureHeader.componentsNumber);
                for (size_t band = 0; band < bandPerComponent; band++) {
                    for (size_t component = 0; component < pictureHeader.componentsNumber; component++) {

                        size_t i = bandPerComponent * component + band;
                        precinctHeader.bandCodingmMode.push_back(
                                readBitsFromBitstream<uint8_t>(blockOfMemory.bitstream, 2)); //
                    }
                }
                precinctHeader.padding = readFromBitsreamAndSwap<uint8_t>(blockOfMemory.bitstream);
            }
            //read packet header
            PacketHeader packetHeader;
            {
                bool pktHdrSizeShort = (pictureHeader.frameWidth * pictureHeader.componentsNumber < 32768) &&
                                       (pictureHeader.verticalWaveletLevels < 3);

                packetHeader.SizeOfData = readBitsFromBitstream<uint32_t>(blockOfMemory.bitstream,
                                                                          pktHdrSizeShort ? PKT_HDR_DATA_SIZE_SHORT
                                                                                          : PKT_HDR_DATA_SIZE_LONG);

                packetHeader.sizeOfTheBitplaneCountSubpacket = readBitsFromBitstream<uint32_t>(blockOfMemory.bitstream,
                                                                                               pktHdrSizeShort
                                                                                               ? PKT_HDR_GCLI_SIZE_SHORT
                                                                                               : PKT_HDR_GCLI_SIZE_LONG);

                packetHeader.signSubpacketSize = readBitsFromBitstream<uint16_t>(blockOfMemory.bitstream,
                                                                                 pktHdrSizeShort
                                                                                 ? PKT_HDR_SIGN_SIZE_SHORT
                                                                                 : PKT_HDR_SIGN_SIZE_LONG);

            }
            //read packet body
            PacketBody packetBody;
            {

            }
            break;
        }
    }


}

