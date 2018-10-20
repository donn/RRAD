#ifndef _vectorize_h
#define _vectorize_h

#include <vector>
#include <string>

#include "Types.h"

namespace RRAD {
    inline std::vector<uint8> vectorize(std::string string) {
        const char* cString = string.c_str();
        return std::vector<uint8>(cString, cString + strlen(cString) + 1);
    }
    inline std::string devectorizeToString(std::vector<uint8> bytes) {
        return std::string(bytes.begin(), bytes.end());
    }
}

#endif