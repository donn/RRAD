#ifndef _packet_h
#define _packet_h

#include "Types.h"
#include <string>
#include <vector>
#include <optional>

namespace RRAD {
    class Packet {
        std::vector<uint8> internalData;
        uint16 acknowledgement;
        uint16 sequence;
    public:
        Packet();
        Packet(std::vector<uint8> data, std::optional<Packet> following = std::nullopt);
        
        static Packet unpacking(std::vector<uint8> data);

        uint16 seq() { return sequence; }
        uint16 ack() { return acknowledgement; }
        std::vector<uint8> body() { return internalData; }

        std::vector<uint8> packed();
        Packet acknowledge();
        bool confirmAcknowledgement(Packet acknowledgementPacket);
    };
}

#endif // _packet_h