#include "Connection.h"
#include "Packet.h"

#include <thread>
#include <iostream>

#define forever for(;;)

RRAD::Connection::Connection(std::string ip, uint16 port, int timeout, uint16 localPort) {
    socket = UDPSocket(ip, port, localPort);
    socket.setTimeout(timeout / 1000, timeout % 1000);
}

std::vector<uint8> RRAD::Connection::read() {
    std::vector<uint8> assembled;
    std::vector<uint8> data;
    Packet packet;

    std::string ip;
    uint16 port;

    uint16 lastAck = 0;
    do {
        data = socket.read(&ip, &port);
        socket.setPeerAddress(ip, port);
        packet = Packet::unpacking(data);
        Packet acknowledgement = packet.acknowledge();

        if (packet.seq() != lastAck && !(packet.seq() == 0xFFFF && packet.ack() == 0xFFFF)) {
            throw "Out of order.";
        }

        socket.write(acknowledgement.packed());
        lastAck = acknowledgement.ack();
        
        auto unpackedData = packet.body();
        assembled.insert(std::end(assembled), std::begin(unpackedData), std::end(unpackedData));

    } while (packet.seq() != 0xFFFF || packet.ack() != 0xFFFF);
    return assembled;
}

void RRAD::Connection::write(std::vector<uint8> data) {
    int transmissions = data.size() / MESSAGE_LENGTH;
    if (data.size() % MESSAGE_LENGTH) {
        transmissions += 1;
    }
    std::vector<uint8> sendable;
    std::optional<Packet> lastPacket = std::nullopt;
    
    for (int i = 0; i <= transmissions; i += 1) {
        Packet newPacket;

        if (i != transmissions) {
            std::vector<uint8>::const_iterator beginning = data.begin() + i * MESSAGE_LENGTH;
            std::vector<uint8>::const_iterator end = data.begin() + (i + 1) * MESSAGE_LENGTH;
            if (end > data.end()) {
                end = data.end();
            }
            sendable = std::vector<uint8>(beginning, end);
            newPacket = Packet(sendable, lastPacket);
        } else {
            newPacket = Packet::terminator();
        }
        
        socket.write(newPacket.packed());
        Packet acknowledgement = Packet::unpacking(socket.read());
        bool confirmedAcknowledgement = newPacket.confirmAcknowledgement(acknowledgement);
        if (!confirmedAcknowledgement) {
            throw "Out of order.";
        }
        lastPacket = newPacket;
    }
}

void RRAD::Connection::listen(std::function<void(Connection)> operativeLoop) {
    forever {
        std::string ip;
        uint16 port;
        std::vector<uint8> data;
        try {
            data = socket.read(&ip, &port);
        } catch (std::exception& exception) {
            continue;
        }

        Packet packet;
        try {
            packet = Packet::unpacking(data);
        } catch (std::exception& exception) {
            std::cerr << "Packet unmarshaling failure, ignoring." << std::endl;
            continue;
        }

        if (packet.ack() == 0 && packet.seq() == 0 && packet.body().size() == 0) {
            auto connection = Connection(ip, port, timeout);
            connection.socket.write(packet.packed());

            std::thread task([&]() {
                operativeLoop(connection);
            });
            task.detach();
        } else {
            std::cerr << "Invalid connection start message." << std::endl;
            continue;
        }
#ifdef _INCREDIBLY_STUPID_ALTSOCKET_TESTING
        forever{}
#endif //_INCREDIBLY_STUPID_ALTSOCKET_TESTING
    }
}
