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
        std::cerr << "Failed to create socket." << std::endl;
        throw errno;
	}
	
    if (bind(sock, myAddr_cast, sizeof(struct sockaddr)) < 0) {
        std::cerr << "Failed to bind." << std::endl;
        throw errno;
    }

	socklen_t leng = sizeof(myAddr_cast);
	//retrieve it from the system to see what port was actually bound to it
	if (getsockname(sock, myAddr_cast, &leng) < 0){
        std::cerr << "Socket not found." << std::endl;
        throw errno;
	}

	//reassign the port to the actual bound port
	myPort = ntohs(myAddr.sin_port);
    std::cout << "RRAD: Bound to receive from " << myAddress << ":" << myPort << std::endl;

	//better than leaving it empty
	setPeerAddress(_peerAddress, _peerPort);
}

RRAD::UDPSocket::UDPSocket() {
	initSocket("0.0.0.0", 0, 0);
}

RRAD::UDPSocket::UDPSocket(std::string _peerAddress, uint16 _peerPort, uint16 _myPort) {
	initSocket(_peerAddress, _peerPort, _myPort);
}

void RRAD::UDPSocket::setPeerAddress(std::string _peerAddress, int _peerPort){
	peerAddress = _peerAddress;
	peerPort = _peerPort;
    peerAddr.sin_family = AF_INET;
    peerAddr.sin_port = htons(peerPort);
    peerAddr.sin_addr.s_addr = inet_addr(peerAddress.c_str());
}

//negative return values indicate failures
void RRAD::UDPSocket::write(std::vector<uint8> buffer){
	int msgLength = buffer.size();
	std::cerr  << "msgLength = " << msgLength << " bytes\n";

	if (msgLength > MESSAGE_LENGTH) {
		throw "write.tooLargeForRRAD";
	} else { //attempt to send
		int number_bytes_sent = sendto(sock, &buffer[0], msgLength, 0, peerAddr_cast, sizeof(struct sockaddr));
		std::cerr  << " sendto sent " << number_bytes_sent << " bytes\n";
		if (number_bytes_sent < 0) {\
			switch(errno){
				case EMSGSIZE:
					throw "write.tooLargeForSocket";
					break;
					//more errors here as needed
				default:
					throw "error.unknown";
			}
		} else if (number_bytes_sent < msgLength) {
			throw "write.partialWrite";
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
	std::cerr  << " recfrom got " << number_bytes_read << " bytes\n";
	if (number_bytes_read < 0){ //==0??
		switch (errno){
			case ETIMEDOUT:
			case EAGAIN:
				throw("Reading timed out\n");
				break;
			case ENOTSOCK:
				throw("The socket arg does not refer to a socket\n");
				break;
			case EIO:
				throw("IO error\n");
				break;
			case ENOBUFS :
				throw("Insufficient resources to receive\n");
				break;
			default:
				std::cerr << "code: " << errno << std::endl;
				std::cerr << '\n' << EINVAL << " " << ENOMEM << " " << EFAULT <<  " " << ECONNREFUSED << " " << EAGAIN << " " << EWOULDBLOCK << ' ' << EBADF << ' ' << ENOTSOCK << '\n';
				throw ( "Check errnos of recvfrom\n"); 
		}
	}

	std::vector<uint8> ret_vector(buffer, buffer+number_bytes_read);
	delete [] buffer;
	
	newPeerIP->assign(inet_ntoa(newPeerAddr.sin_addr));
	*newPeerPort = ntohs(newPeerAddr.sin_port);
	return ret_vector;
}

RRAD::UDPSocket::~UDPSocket() {
    //pthread_mutex_unlock(&mutex);
    if (sock > 0) {
        close(sock);
    }
}
#endif
