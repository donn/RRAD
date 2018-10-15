#ifndef _udpsocket_h
#define _udpsocket_h

// CPP STL
#include <string>
#include <vector>

// POSIX
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

// Project
#include "Types.h"

#ifdef _INCREDIBLY_STUPID_ALTSOCKET_TESTING
#define MESSAGE_LENGTH 4
#else
#define MESSAGE_LENGTH	1024
#endif
#define BUFFER_SIZE 	8

namespace RRAD {
    class UDPSocket {
        friend class Connection;
        int sock;

        //"my" address/port refers to the address/port the socket can receive from
        //for a server: 0.0.0.0 accepts connections coming to all interfaces
        std::string myAddress;
        int myPort;
        sockaddr_in myAddr;
        sockaddr* myAddr_cast;
        
        std::string peerAddress;
        int peerPort;
        sockaddr_in peerAddr;
        sockaddr* peerAddr_cast;
		

    public:
        void initSocket(std::string address, uint16 port, uint16 localPort);

        UDPSocket();
        UDPSocket(std::string address, uint16 port, uint16 localPort = 0);
        ~UDPSocket();

        std::vector<uint8> read(std::string* returnedIP = NULL, uint16* returnedPort = NULL);
        void write(std::vector<uint8> data);
        
        void setTimeout(int timeout_s, int timeout_ms);
        void setPeerAddress(std::string _peerAddress, uint16 _peerPort, bool reinitialized = true);

        uint16 getLocalPort();
        uint16 getRemotePort();
        std::string getRemoteIP();

    };
}
#endif // _udpsocket_h
