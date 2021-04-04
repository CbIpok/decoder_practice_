#include <benchmark/benchmark.h>
#include "bitstream.h"
#include <cassert>

static void BM_SomeFunction(benchmark::State& state) {
    // Perform setup here
    for (auto _ : state) {
        // This code gets timed
        {
            std::vector<uint8_t> testVector;
            size_t testSize = 10e5;
            testVector.resize(testSize);
            for (size_t i = 0; i < testSize; i++) {
                testVector[i] = i;
            }
            FifoBuf buf;
            buf.recive(NetworkPacket(testVector.data(),testSize));
            for (size_t i = 0; i < testSize; i++) {
                assert((uint8_t)i == buf.readBits(sizeof(uint8_t)*CHAR_BIT));
            }
        }
    }

}
// Register the function as a benchmark
BENCHMARK(BM_SomeFunction);
// Run the benchmark
BENCHMARK_MAIN();
