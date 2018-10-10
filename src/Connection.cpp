#include "Connection.h"
#include "Packet.h"

#include <thread>
#include <iostream>

#define forever for(;;)

RRAD::Connection::Connection(std::string ip, uint16 port, int timeout) {
    socket = UDPSocket(ip, port);
}

std::vector<uint8> RRAD::Connection::read() {
    std::vector<uint8> assembled;
    std::vector<uint8> data;
    Packet packet;

    data = socket.read(timeout);
    packet = Packet(data);
    while (packet.seq() != 0xFFFF && packet.ack() != 0xFFFF) {
        assembled.insert(std::end(assembled), std::begin(data), std::end(data));
        data = socket.read(timeout);
        packet = Packet(data);
        socket.write(packet.acknowledge().raw());
    }
    return assembled;
}

void RRAD::Connection::write(std::vector<uint8> data) {
    int transmissions = data.size() / MESSAGE_LENGTH;
    if (data.size() % MESSAGE_LENGTH) {
        transmissions += 1;
    }
    std::vector<uint8> sendable;
    for (int i = 0; i < transmissions; i += 1) {
        std::vector<uint8>::const_iterator beginning = data.begin() + i * 1024;
        std::vector<uint8>::const_iterator end = data.begin() + (i + 1) * 1024;
        if (end > data.end()) {
            end = data.end();
        }
        sendable = std::vector<uint8>(beginning, end);
        // TO-DO: Sending
    }
}

void RRAD::Connection::listen(std::function<void(Connection, std::vector<uint8>)> operativeLoop) {
    forever {
        std::string ip;
        uint16 port;
        std::vector<uint8> data;
        try {
            data = socket.read(timeout, &ip, &port);
        } catch (std::exception& exception) {
            continue;
        }

        Packet packet;
        try {
            packet = Packet(data);
        } catch (std::exception& exception) {
            std::cerr << "Packet unmarshaling failure, ignoring." << std::endl;
            continue;
        }

        auto connection = Connection(ip, port, timeout);
        connection.socket.write(packet.acknowledge(connection.socket.getLocalPort()).raw());

        std::thread task([&]() {
            operativeLoop(connection, connection.read());
        });
    }
}

