
#include <string>
#include <gtest/gtest.h>
#include "bitstream.h"

TEST(BitReader, uint8t_test) {
    std::vector<uint8_t> testVector;
    size_t testSize = 10e5;
    testVector.resize(testSize);
    for (size_t i = 0; i < testSize; i++) {
        testVector[i] = i;
    }
    FifoBuf buf;
    buf.recive(NetworkPacket(testVector.data(),testSize));
    for (size_t i = 0; i < testSize; i++) {
        ASSERT_EQ((uint8_t)i,buf.readBits(sizeof(uint8_t)*CHAR_BIT));
    }
    ASSERT_EQ(1 , 1);
}

int main(int argc, char *argv[]){
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
