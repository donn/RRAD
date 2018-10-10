#include "Packet.h"

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

static int assemble(uint8* pointer, int count) {
    int assembled = 0;
    for (int i = 0; i < count; i += 1) {
        assembled |= pointer[i] << (8 * i);
    }
    return assembled;
}

RRAD::Packet RRAD::Packet::unpacking(std::vector<uint8> data) {
    Packet packet = Packet();
    packet.sequence = assemble(&data[0], 2);
    packet.acknowledgement = assemble(&data[2], 2);

    auto beginning = data.begin() + 4;
    auto end = data.end();

    packet.internalData = std::vector<uint8>(beginning, end);
}

static std::vector<uint8> disassemble(int value) {
    std::vector<uint8> bytes = std::vector<uint8>();
    while (value) {
        bytes.push_back(value & 0xFF);
        value >>= 8;
    }
    return bytes;
}

std::vector<uint8> RRAD::Packet::packed() {
    std::vector<uint8> data = internalData.size + 4;

    auto packedSequence = disassemble(sequence);
    auto packedAcknowledgement = disassemble(acknowledgement);
    
    data.insert(data.end(), packedSequence.begin(), packedSequence.end());
    data.insert(data.end(), packedAcknowledgement.begin(), packedAcknowledgement.end());
    data.insert(data.end(), internalData.begin(), internalData.end());

    return data;
}

RRAD::Packet RRAD::Packet::acknowledge() {
    Packet packet = Packet();
    packet.sequence = acknowledgement;
    packet.acknowledgement = sequence + internalData.size();
    packet.internalData = std::vector<uint8>(0);
    return packet;
}

bool RRAD::Packet::confirmAcknowledgement(Packet acknowledgementPacket) {
    return (acknowledgementPacket.ack() == sequence + internalData.size());
}