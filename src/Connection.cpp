#include "Connection.h"
#include "Packet.h"

#include <thread>
#include <iostream>

#define forever for(;;)

#define CONNECTION_DESCRIPTOR(stream) do { \
	stream << "Connection $" << this << " to (@" << socketp->myPort << " -> " << socketp->peerAddress << ":" << socketp->peerPort << ")"; \
} while (0);

#define CONNECTION_ERROR(message) do { \
	std::cerr << "ERROR: "; \
	CONNECTION_DESCRIPTOR(std::cerr); \
	std::cerr << ": " << message << std::endl; \
	throw message; \
} while (0);

RRAD::Connection::Connection(std::string ip, uint16 port, int timeout, uint16 localPort) {
    this->ip = ip;
    this->port = port;
    this->timeout = timeout;
	socketp = new UDPSocket(ip, port, localPort);
	socketp->setTimeout(timeout / 1000, timeout % 1000);
}

RRAD::Connection::~Connection() {
	delete socketp;
}

std::vector<uint8> RRAD::Connection::read() {
    std::vector<uint8> assembled;
    std::vector<uint8> data;
    Packet packet;

    std::string ip;
    uint16 port;

    uint16 lastAck = 0;
    do {
        data = socketp->read(&ip, &port);
        socketp->setPeerAddress(ip, port);
        packet = Packet::unpacking(data);
        Packet acknowledgement = packet.acknowledge();

        if (packet.seq() != lastAck && !(packet.seq() == 0xFFFF && packet.ack() == 0xFFFF)) {
            CONNECTION_ERROR("conn.outOfOrder");
        }

        socketp->write(acknowledgement.packed());
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

    std::string newIP;
    uint16 newPort;
    auto initializer = Packet::initializer();
    socketp->write(initializer.packed());
    auto initializerReply = Packet::unpacking(socketp->read(&newIP, &newPort));
    if (!initializer.confirmAcknowledgement(initializerReply)) {
        CONNECTION_ERROR("conn.deniedHandshake");
    }
    socketp->setPeerAddress(newIP, newPort);
    
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
        
        socketp->write(newPacket.packed());
        Packet acknowledgement = Packet::unpacking(socketp->read());
        bool confirmedAcknowledgement = newPacket.confirmAcknowledgement(acknowledgement);
        if (!confirmedAcknowledgement) {
            CONNECTION_ERROR("conn.outOfOrder");
        }
        lastPacket = newPacket;
    }
}

void RRAD::Connection::listen(std::function<void(Connection&)> operativeLoop) {
    forever {
        std::string ip;
        uint16 port;
        std::vector<uint8> data;
        try {
            data = socketp->read(&ip, &port);
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

            std::vector<uint8> echoBack = packet.packed();
            std::thread task([=]() { //copying not referencing; after the detach, everything is lost
                Connection connection(ip, port, timeout);
                connection.socketp->write(echoBack);
                operativeLoop(connection);
            });
            task.detach();
        } else {
            std::cerr << "Invalid handshake from " << ip << ":" << port << " ignoring." << std::endl;
            continue;
        }
#ifdef _INCREDIBLY_STUPID_ALTSOCKET_TESTING
        forever{}
#endif //_INCREDIBLY_STUPID_ALTSOCKET_TESTING
    }
}
