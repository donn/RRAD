#ifndef _packet_h
#define _packet_h

#include "Types.h"
#include <string>
#include <vector>

namespace RRAD {
    class Packet {
    public:
        Packet();
        Packet(std::vector<uint8> data);
        uint16 seq();
        uint16 ack();
        std::vector<uint8> body();
        std::vector<uint8> raw();
        Packet acknowledge();
    };
}

#endif // _packet_h