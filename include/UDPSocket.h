#ifndef _udpsocket_h
#define _udpsocket_h

#include <string>
#include <vector>

#include "Types.h"

#define MESSAGE_LENGTH	1024
#define BUFFER_SIZE 	8

namespace RRAD {
    class UDPSocket {
    public:
        UDPSocket() {};
        UDPSocket(std::string address, uint16 port);

        std::vector<uint8> read(int timeout, std::string* returnedIP = NULL, uint16* returnedPort = NULL);
        void write(std::vector<uint8> data);

        uint16 getLocalPort();
        uint16 getRemotePort();
        std::string getRemoteIP();

    };
}
#endif