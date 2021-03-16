#pragma once

#include <string>
#include <vector>
#include <memory>
#include "bitstream.h"
#include "types.h"

class BlockOfMemory final// struct with destructure
{
public:
    BlockOfMemory(uint8_t *data,size_t size);

    BlockOfMemory(const BlockOfMemory&) = delete;

    BlockOfMemory(BlockOfMemory&& blockOfMemory) noexcept;

    ~BlockOfMemory() = default;

    std::unique_ptr<uint8_t[]> data;
    std::size_t len;
    Bitstream bitstream;
};

class BlockParser final
{
public:

    BlockParser(Bitstream&& bitstream);

    BlockParser(uint8_t *bitstream, size_t size);

    BlockOfMemory getHeader(); //todo SLICE HEADER LEN IS NOT CONSTANT!!
    
    std::vector<BlockOfMemory> getSlices();

    BlockOfMemory getSlice();

    BlockOfMemory gesEos(); //to simply 

private:
    constexpr static size_t HEADER_SIZE = 96;

    Bitstream bitstream;

    std::unique_ptr<uint8_t> buf;

    enum class ParseState: uint8_t
    {
        HEADER,
        SLISES,
        EOS,
        DONE
    };

    size_t getSliceSize(Bitstream& lbitsream);
    

   
};

<<<<<<< HEAD




class DetailParser 
=======
class DetailParser final
>>>>>>> 6835424e78c01eed3717087e323f3c69f89f04fd
{
public:
    DetailParser() = default;
    ~DetailParser() = default;

    void parseHeader(BlockOfMemory& blockOfMemory);

    const PictureHeader& getPih() { return pictureHeader; };
    
    const std::vector<Component>& getComponentTable() { return componentTable; };

    const std::vector<Band>& getWeightTable() { return weightTable; };
private:

<<<<<<< HEAD
    PictureHeader pictureHeader;
    std::vector<Component> componentTable;
    std::vector<Band> weightTable;



    

=======
>>>>>>> 6835424e78c01eed3717087e323f3c69f89f04fd
};

//methods
void writeBlockOfMemoryToFile(const BlockOfMemory& blockOfMemory, const std::string& fileName);