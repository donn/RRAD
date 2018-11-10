#include "Packet.h"

RRAD::Packet::Packet() {}
RRAD::Packet::Packet(std::vector<uint8> data, std::optional<Packet> following) {
    internalData = data;
    if (following.has_value()) {
        auto predecessor = following.value();
        acknowledgement = predecessor.ack();
        sequence = predecessor.seq() + predecessor.body().size();
    } else {
        acknowledgement = 0;
        sequence = 0;
    }
}

static SEQACK_T assemble(uint8* pointer, int count) {
    SEQACK_T assembled = 0;
    for (int i = 0; i < count; i += 1) {
        assembled |= pointer[i] << (8 * i);
    }
    return assembled;
}

RRAD::Packet RRAD::Packet::unpacking(std::vector<uint8> data) {
    Packet packet = Packet();
    packet.sequence = assemble(&data[0], SEQACK_LENGTH);
    packet.acknowledgement = assemble(&data[SEQACK_LENGTH], SEQACK_LENGTH);

    auto beginning = data.begin() + PACKET_OVERHEAD;
    auto end = data.end();

    packet.internalData = std::vector<uint8>(beginning, end);
    return packet;
}

RRAD::Packet RRAD::Packet::initializer() {
    Packet packet = Packet();
    packet.sequence = 0x0;
    packet.acknowledgement = 0x0;
    packet.internalData = {};
    return packet;
}

bool RRAD::Packet::isInitializer() {
    return sequence == 0x0 && acknowledgement == 0x0;
}

RRAD::Packet RRAD::Packet::nack() {
    Packet packet = Packet();
    packet.sequence = SEQACK_MAX-1;
    packet.acknowledgement = SEQACK_MAX-1;
    packet.internalData = {};
    return packet;
}

bool RRAD::Packet::isNack() {
    return sequence == SEQACK_MAX-1 && acknowledgement == SEQACK_MAX-1;
}

RRAD::Packet RRAD::Packet::terminator() {
    Packet packet = Packet();
    packet.sequence = SEQACK_MAX;
    packet.acknowledgement = SEQACK_MAX;
    packet.internalData = {};
    return packet;
}

bool RRAD::Packet::isTerminator() {
    return sequence == SEQACK_MAX && acknowledgement == SEQACK_MAX;
}

static std::vector<uint8> disassemble(SEQACK_T value, int count) {
    std::vector<uint8> bytes = std::vector<uint8>();
    while (count--) {
        bytes.push_back(value & 0xFF);
        value >>= 8;
    }
    return bytes;
}

std::vector<uint8> RRAD::Packet::packed() {
    std::vector<uint8> data;

    auto packedSequence = disassemble(sequence, SEQACK_LENGTH);
    auto packedAcknowledgement = disassemble(acknowledgement, SEQACK_LENGTH);

    data.insert(data.end(), packedSequence.begin(), packedSequence.end());
    data.insert(data.end(), packedAcknowledgement.begin(), packedAcknowledgement.end());
    data.insert(data.end(), internalData.begin(), internalData.end());

    return data;
}

RRAD::Packet RRAD::Packet::acknowledge() {
    Packet packet = Packet();
    packet.sequence = sequence; //gonna use that
    packet.acknowledgement = sequence + internalData.size();
    packet.internalData = std::vector<uint8>(0);
    return packet;
}
bool RRAD::Packet::confirmAcknowledgement(Packet acknowledgementPacket) {
    return (acknowledgementPacket.ack() == sequence + internalData.size());
}
