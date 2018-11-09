#include "Types.h"
#include <cstdlib>
#include <random> 
extern "C" {
    // For TweetNACL
    void randombytes(unsigned char* buffer, unsigned long long count) {
        static std::random_device generator;
        static std::uniform_int_distribution<unsigned char> distribution(0,255);
        for (unsigned long long i = 0; i < count; i += 1) {
            buffer[i] = distribution(generator);
        }
    }
}