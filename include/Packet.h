#ifndef _packet_h
#define _packet_h

#include "Types.h"
#include <string>
#include <vector>
#include <optional>

namespace RRAD {
    class Packet {
    public:
        Packet();
        Packet(std::vector<uint8> data, std::optional<Packet> predecessor = std::nullopt);
        
        static Packet unmarshalling(std::vector<uint8> data);

        uint16 seq();
        uint16 ack();
        std::vector<uint8> body();
        std::vector<uint8> raw();
        Packet acknowledge(uint16 port = 0);
        bool confirmAcknowledgement(Packet acknowledgement);
    };
}

#endif // _packet_h