#include "Types.h"
#include <cstdlib>
#include <random> 
extern "C" {
    // For TweetNACL
    void randombytes(uint8* buffer, uint64 count) {
        static std::random_device generator;
        static std::uniform_int_distribution<uint8> distribution(0,255);
        for (uint64 i = 0; i < count; i += 1) {
            buffer[i] = distribution(generator);
        }
    }
}