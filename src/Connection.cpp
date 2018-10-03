#include "Connection.h"
#include "Packet.h"

#include <thread>

#define forever for(;;)

RRAD::Connection::Connection(std::string ip, uint16 port, int timeout) {
    socket = UDPSocket(ip, port);
}

void RRAD::Connection::listen(std::function<void(Connection, std::vector<uint8>)> operativeLoop) {
    forever {
        std::string ip;
        uint16 port;
        std::vector<uint8> data = socket.read(timeout, &ip, &port);

        Packet packet;
        try {
            packet = Packet(data);
        } catch (std::exception& exception) {
            continue;
        }

        std::vector<uint8> assembled;
        auto connection = Connection(ip, port, timeout);

        while (packet.seq() != 0xFFFF && packet.ack() != 0xFFFF) {
            assembled.insert(std::end(assembled), std::begin(data), std::end(data));
            connection.socket.write(packet.acknowledge().raw());
            data = connection.socket.read(timeout);
            packet = Packet(data);
        }
        connection.socket.read(timeout);

        std::thread task([&]() {
            operativeLoop(connection, data);
        });
    }
}

