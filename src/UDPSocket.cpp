#ifndef _INCREDIBLY_STUPID_ALTSOCKET_TESTING
// Project
#include "UDPSocket.h"

// CPP STL
#include <string>
#include <iostream>

// POSIX
#include <unistd.h> 
#include <pthread.h>
#include <arpa/inet.h>

#define SOCKET_DESCRIPTOR(stream) do { \
	stream << "[UDPSocket $" << this << " to (@" << myPort << " -> " << peerAddress << ":" << peerPort << ", fd " << sock << ")] "; \
} while (0);

#define SOCKET_ERROR(message) do { \
	std::cerr << "ERROR: "; \
	SOCKET_DESCRIPTOR(std::cerr); \
	std::cerr << message << std::endl; \
	throw message; \
} while (0);

#ifndef _VERBOSE_UDPSOCKET_DEBUG
#define _VERBOSE_UDPSOCKET_DEBUG 0
#endif


void RRAD::UDPSocket::initSocket(std::string _peerAddress, uint16 _peerPort, uint16 _myPort) {
    myAddr_cast = (sockaddr*)&myAddr;
    peerAddr_cast = (sockaddr*)&peerAddr;

	myAddress = "0.0.0.0";
	myPort = _myPort;

    myAddr.sin_family = AF_INET;
    myAddr.sin_port = htons(myPort);
    myAddr.sin_addr.s_addr = inet_addr(myAddress.c_str());

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        SOCKET_ERROR("socket.failedToCreate");
	}
	
    if (bind(sock, myAddr_cast, sizeof(struct sockaddr)) < 0) {
        SOCKET_ERROR("socket.failedToBind");
    }

	socklen_t leng = sizeof(myAddr_cast);
	//retrieve it from the system to see what port was actually bound to it
	if (getsockname(sock, myAddr_cast, &leng) < 0){
        SOCKET_ERROR("socket.notFound");
	}

	//reassign the port to the actual bound port
	myPort = ntohs(myAddr.sin_port);

	//better than leaving it empty
	setPeerAddress(_peerAddress, _peerPort, false);
}

RRAD::UDPSocket::UDPSocket() {
	initSocket("0.0.0.0", 0, 0);
}

RRAD::UDPSocket::UDPSocket(std::string _peerAddress, uint16 _peerPort, uint16 _myPort) {
	initSocket(_peerAddress, _peerPort, _myPort);
}

void RRAD::UDPSocket::setPeerAddress(std::string _peerAddress, uint16 _peerPort, bool reinitialized) {
	peerAddress = _peerAddress;
	peerPort = _peerPort;
    peerAddr.sin_family = AF_INET;
    peerAddr.sin_port = htons(peerPort);
    peerAddr.sin_addr.s_addr = inet_addr(peerAddress.c_str());

	if (_VERBOSE_UDPSOCKET_DEBUG) {
		std::cout << (reinitialized? "Reinitialized ": "Initialized ");
		SOCKET_DESCRIPTOR(std::cout);
		std::cout << std::endl;
	}
}

//negative return values indicate failures
void RRAD::UDPSocket::write(std::vector<uint8> buffer){
	int msgLength = buffer.size();

	if (_VERBOSE_UDPSOCKET_DEBUG) {
		SOCKET_DESCRIPTOR(std::cout);
		std::cerr  << "Trying to write: msgLength = " << msgLength << " bytes\n";
	}

	if (msgLength > MESSAGE_LENGTH) {
		std::cerr << "Write too large." << std::endl;
		throw "write.tooLargeForRRAD";
	} else { //attempt to send
		int number_bytes_sent = sendto(sock, &buffer[0], msgLength, 0, peerAddr_cast, sizeof(struct sockaddr));
		if (_VERBOSE_UDPSOCKET_DEBUG) {
			SOCKET_DESCRIPTOR(std::cout);
			std::cerr  << "sendto sent " << number_bytes_sent << " bytes\n";
		}
		if (number_bytes_sent < 0) {\
			switch(errno){
				case EMSGSIZE:
					SOCKET_ERROR("write.tooLargeForSocket");
					break;
					//more errors here as needed
				default:
					SOCKET_ERROR("write.unknown");
			}
		} else if (number_bytes_sent < msgLength) {
			SOCKET_ERROR("write.partial");
		}
	}
}

void RRAD::UDPSocket::setTimeout(int timeout_s, int timeout_ms){
	struct timeval timeout_dur;
	timeout_dur.tv_sec = timeout_s;
	timeout_dur.tv_usec = timeout_ms*1000;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_dur, sizeof(timeout_dur));
}

std::vector<uint8> RRAD::UDPSocket::read(std::string *newPeerIP, uint16 *newPeerPort){
	char *buffer = new char[MESSAGE_LENGTH];

	sockaddr_in newPeerAddr;
	sockaddr *newPeerAddr_cast = (sockaddr *)&newPeerAddr;
	socklen_t leng = sizeof(newPeerAddr_cast);
	int number_bytes_read = recvfrom(sock, buffer, MESSAGE_LENGTH, 0, newPeerAddr_cast, &leng);
	if (_VERBOSE_UDPSOCKET_DEBUG) {
		SOCKET_DESCRIPTOR(std::cout);
		std::cout  << "recvfrom got " << number_bytes_read << " bytes" << std::endl;
	}
	if (number_bytes_read < 0){ //==0??
		switch (errno){
			case ETIMEDOUT:
			case EAGAIN:
				SOCKET_ERROR("socket.read.timeout");
				break;
			case ENOTSOCK:
				SOCKET_ERROR("socket.read.notASocket");
				break;
			case EIO:
				SOCKET_ERROR("socket.read.ioError");
				break;
			case EBADF:
				SOCKET_ERROR("socket.read.badFileDescriptor");
				break;
			case ENOBUFS :
				SOCKET_ERROR("socket.read.insufficientResources");
				break;
			default:
				std::cerr << "Error code: " << errno << std::endl;
				SOCKET_ERROR("read.unknown"); 
		}
	}

	std::vector<uint8> ret_vector(buffer, buffer+number_bytes_read);
	delete [] buffer;

	if (newPeerIP) {
		*newPeerIP = std::string(inet_ntoa(newPeerAddr.sin_addr));
	}
	if (newPeerPort) {
		*newPeerPort = ntohs(newPeerAddr.sin_port);
	}
	return ret_vector;
}

RRAD::UDPSocket::~UDPSocket() {
    if (sock > 0) {
        close(sock);
    }
}
#endif
