#ifndef UDPSOCKET_H
#define UDPSOCKET_H

#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include "common.hpp" //rem

class UDPSocket {
protected:
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

	//initializes the socket to receive from _myAddress on _myPort
    void initializeSocket(std::string _myAddress, int _myPort);

public:
	//by default peer address and my address are the same
    UDPSocket(std::string _myAddress, int _myPort);

	//unless specified in the constructor:
    UDPSocket(std::string _myAddress, int _myPort, std::string _peerAddress, int _peerPort);

	//by default ip = "0.0.0.0", receivingPort = 0
    UDPSocket();

	//-===================-
	//Socket Configuration
	//-===================-
	//sets the destination address
    void setPeerAddress(std::string _peerAddress, int _peerPort);

	//better to specify it beforehand; 0 means blocking
    void setReadTimeoutDuration(int timeout_s, int timeout_ms);


	//-===================-
	//Main operations:
	//-===================-
    bool write(std::vector<uint8> buffer);

	//would get a better performance using references but leave it for now
	std::vector<uint8> read(std::string *newPeerIP, int *newPeerPort);

	//etc
    int getMyPort();
    int getPeerPort();
    ~UDPSocket ( );
};

#endif // UDPSOCKET_H
