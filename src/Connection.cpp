#include "Connection.h"
#include "Packet.h"
#include "Vectorize.h"

#include <thread>
#include <iostream>
#include <cmath>
#include <exception>


#define forever for(;;)

#define CONNECTION_DESCRIPTOR(stream) do { \
	stream << "Connection $" << this << " to (@" << socketp->myPort << " -> " << socketp->peerAddress << ":" << socketp->peerPort << ")"; \
} while (0);

#define CONNECTION_ERROR(message) do { \
	std::cerr << "[RRAD] ERROR: "; \
	CONNECTION_DESCRIPTOR(std::cerr); \
	std::cerr << ": " << message << std::endl; \
	throw std::runtime_error(message); \
} while (0);

const auto TRUE_MESSAGE_LENGTH = MESSAGE_LENGTH - PACKET_OVERHEAD;

#define i2byteStartBoundary(i) ((i)*TRUE_MESSAGE_LENGTH);
#define byteStartBoundary2i(bsb) (bsb/TRUE_MESSAGE_LENGTH);

RRAD::Connection::Connection(std::string ip, uint16 port, int timeout, uint16 localPort) {
    this->ip = ip;
    this->port = port;
    this->timeout = timeout;
	socketp = std::shared_ptr<UDPSocket>(new UDPSocket(ip, port, localPort));
	socketp->setTimeout(timeout / 1000, timeout % 1000);
}

RRAD::Connection::~Connection() {
}

std::vector<uint8> RRAD::Connection::read() {
    std::vector<uint8> assembled;
    std::vector<uint8> readdata;
    Packet packet;

    std::string ip;
    uint16 port;

    SEQACK_T lastAck = 0;
    int retrials = MAX_RETRIALS;
 
    for (int i = 0; i==0 || (retrials+1 > 0 && !packet.isTerminator()); i += 1) {
        Packet acknowledgement;
        try {
            readdata = socketp->read(&ip, &port);
            socketp->setPeerAddress(ip, port);
            packet = Packet::unpacking(readdata);
            acknowledgement = packet.acknowledge();
        } catch (std::exception& e) {
            std::string eMsg(e.what());
            if (eMsg == "socket.read.timeout") {
                packet = Packet::nack();
            } else {
                CONNECTION_ERROR(eMsg);
            }
        }

        double boundary = double(packet.seq())/double(TRUE_MESSAGE_LENGTH); 
        bool previousPacket = (nearbyint(boundary) == boundary) && boundary < i;
        if (previousPacket){
            i--;
        }

        if (!previousPacket && packet.seq() != lastAck && !packet.isTerminator()) {
            i--;
            retrials--;
            continue;
        }

        socketp->write(acknowledgement.packed());
        lastAck = acknowledgement.ack();

        auto unpackedData = packet.body();
        
        // no need to filter duplicated packets if packets r inserted in the right place
        assembled.insert(std::begin(assembled)+packet.seq(), std::begin(unpackedData), std::end(unpackedData));

    }
    
    if (retrials == -1) {
        CONNECTION_ERROR("conn.read.retrialsexhausted");
    }

    return assembled;
}

void RRAD::Connection::write(std::vector<uint8> data) {
    int transmissions = data.size() / TRUE_MESSAGE_LENGTH;
    int retrials = MAX_RETRIALS; // total trials for all packets (not per packet)
    if (data.size() % TRUE_MESSAGE_LENGTH) {
        transmissions += 1;
    }
    std::vector<uint8> sendable;
    std::optional<Packet> lastPacket = std::nullopt;
    std::vector<uint8> readdata;

    std::string newIP;
    uint16 newPort;
    // handshake: won't retry
    Packet initializer = Packet::initializer();
    socketp->write(initializer.packed());
    Packet initializerReply;
    try {
        readdata = socketp->read(&newIP, &newPort);
        initializerReply = Packet::unpacking(readdata);
    } catch (std::exception& e) {
        std::string eMsg(e.what());
        if (eMsg == "socket.read.timeout") {
            CONNECTION_ERROR("conn.handshake.timeout");
        } else {
            CONNECTION_ERROR(eMsg);
        }
    }
    if (!initializer.confirmAcknowledgement(initializerReply)) {
        CONNECTION_ERROR("conn.deniedHandshake");
    }
    socketp->setPeerAddress(newIP, newPort);

    int i;
        //std::cout << RRAD::devectorizeToString(data) << std::endl;
    for (i = 0; i <= transmissions && (retrials+1 > 0); i += 1) {
        Packet newPacket;
        Packet acknowledgement;
        if (i != transmissions) {
            std::vector<uint8>::const_iterator beginning = data.begin() + i2byteStartBoundary(i);
            std::vector<uint8>::const_iterator end = data.begin() + i2byteStartBoundary(i+1);
            if (end > data.end()) {
                end = data.end();
            }
            sendable = std::vector<uint8>(beginning, end);
            std::cout << RRAD::devectorizeToString(sendable) << std::endl;

            newPacket = Packet(sendable, lastPacket);
        } else {
            newPacket = Packet::terminator();
        }

        socketp->write(newPacket.packed());

        bool previousAcknowledgment;
        do { //discard previous Acks
            try {
                readdata = socketp->read();
                acknowledgement = Packet::unpacking(readdata);
            } catch (std::exception& e) {
                std::string eMsg(e.what());
                if (eMsg == "socket.read.timeout") {
                    acknowledgement = Packet::nack();
                } else {
                    std::cerr << "lool?";
                    CONNECTION_ERROR(eMsg);
                }
            }

            bool confirmedAcknowledgement = newPacket.confirmAcknowledgement(acknowledgement);
            double boundary = double(acknowledgement.seq())/double(TRUE_MESSAGE_LENGTH); 
            previousAcknowledgment = (nearbyint(boundary) == boundary) && boundary < i;
            if (!confirmedAcknowledgement) {
                acknowledgement = Packet::nack();
            }
        } while (previousAcknowledgment);


        if (acknowledgement.isNack()) {
            i--;
            retrials--;
            continue;
        }
        lastPacket = newPacket;
    }
    if (retrials == -1 && ( !lastPacket.has_value() || (lastPacket.has_value() && !lastPacket.value().isTerminator()))) {
        CONNECTION_ERROR("conn.send.retrialsexhausted");
    }
}

void RRAD::Connection::listen(std::function<void(Connection*)> operativeLoop) {
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
                operativeLoop(&connection);
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
