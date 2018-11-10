#ifndef _packet_h
#define _packet_h

#include "Types.h"
#include <string>
#include <vector>
#include <optional>
#include <errno.h>

#define SEQACK_LENGTH 8
#define SEQACK_T uint64
#define SEQACK_MAX 0xFFFFFFFFFFFFFFFF
#define PACKET_OVERHEAD (SEQACK_LENGTH * 2)

namespace RRAD {
    class Packet {
        std::vector<uint8> internalData;
        SEQACK_T acknowledgement;
        SEQACK_T sequence;
    public:
        Packet();
        Packet(std::vector<uint8> data, std::optional<Packet> following = std::nullopt);
        
        static Packet unpacking(std::vector<uint8> data);
        static Packet initializer();
        bool isInitializer();
        static Packet nack();
        bool isNack();
        static Packet terminator();
        bool isTerminator();

        SEQACK_T seq() { return sequence; }
        SEQACK_T ack() { return acknowledgement; }
        std::vector<uint8> body() { return internalData; }

        std::vector<uint8> packed();
        Packet acknowledge();
        bool confirmAcknowledgement(Packet acknowledgementPacket);
    };
}

#endif // _packet_h
