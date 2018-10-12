

#ifdef _INCREDIBLY_STUPID_ALTSOCKET_TESTING
#include "UDPSocket.h"
#include <iostream>

RRAD::UDPSocket::UDPSocket(std::string address, uint16 port) {
    
}

std::vector<uint8> RRAD::UDPSocket::read(std::string* returnedIP, uint16* returnedPort) {
    if (returnedIP) {
        *returnedIP = "192.168.77.77";
    }
    if (returnedPort) {
        *returnedPort = 69;
    }

    static int sequence = 0;
    switch (sequence++) {
        case 0:
            return {0, 0, 0, 0};
        case 1:
            return {0, 0, 0, 0, 0x4e, 0x69, 0x61};
        case 2:
            return {3, 0, 0, 0, 0x69, 0x61, 0x69, 0x61, 0x69, 0x0};
        case 3:
            return {0xFF, 0xFF, 0xFF, 0xFF};
        case 4:
            return {0, 0, 4, 0};
        case 5:
            return {0, 0, 8, 0};
        case 6:
            return {0xFF, 0xFF, 0xFF, 0xFF};
        default:
            return {0xFF, 0xFF, 0xFF, 0xFF};
    }
}
void RRAD::UDPSocket::write(std::vector<uint8> data) {
    for (int byte : data) {
        std::cout << byte << ",";
    }
    std::cout << std::endl;
}
#endif // _INCREDIBLY_STUPID_ALTSOCKET_TESTING