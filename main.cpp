#include "decoder_low_quality/Parser.h"
#include <cstdint>
#include <iostream>
#include <bitset>
#include "decoder_low_quality/types.h"

#include <array>

int main()
{ 
     //// code not for review
    FILE* fp { nullptr };

    fopen_s(&fp, "1_b_3.jxs", "rb");  // r for read, b for binary

    fseek(fp, 0L, SEEK_END);
    size_t len = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    uint8_t* p = new uint8_t[len];

    fread(p, len, 1, fp);

    BlockParser parser(p,len);

    auto block = parser.getHeader();

    writeBlockOfMemoryToFile(block, "header.jxs");

    DetailParser dparser;

    dparser.parseHeader(block);

    //std::cout << bitCount(picHeader.progressionMode);

    auto slices = parser.getSlices();
    for (auto& slise : slices)
    {
        dparser.parseSlice(slise);
    }

   //fifo test
    FifoBuf buf;
    {
        NetworkPacket packet(block.data.get(),block.len);
        buf.recive(packet);

        dparser.parseHeaderFast(buf);

    }

    return 0;
}