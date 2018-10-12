// Project
#include "udpsocket.hpp"
#include "errors.hpp"
#include "common.hpp"

// C STD
#include <cstring>

// CPP STL
#include <string>
#include <iostream>

// POSIX?
#include <unistd.h> 
#include <pthread.h>
#include <arpa/inet.h> 

void UDPSocket::initializeSocket(std::string _myAddress, int _myPort) {
	myAddress = _myAddress;
	myPort = _myPort;

    myAddr.sin_family = AF_INET;
    myAddr.sin_port = htons(myPort);
    myAddr.sin_addr.s_addr = inet_addr(myAddress.c_str());

    myAddr_cast = (sockaddr*)&myAddr;
    peerAddr_cast = (sockaddr*)&peerAddr;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0){
        std::cerr << "Failed to create socket." << std::endl;
        throw errno;
	}

    std::cout << "Binding..." << std::endl;
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
    std::cout << "Bound to receive from" << myAddress << ":" << myPort << std::endl;

	//better than leaving it empty
	setPeerAddress(myAddress, myPort);
}

void UDPSocket::setPeerAddress(std::string _peerAddress, int _peerPort){
	peerAddress = _peerAddress;
	peerPort = _peerPort;
    peerAddr.sin_family = AF_INET;
    peerAddr.sin_port = htons(peerPort);
    peerAddr.sin_addr.s_addr = inet_addr(peerAddress.c_str());
}

UDPSocket::UDPSocket(std::string _myAddress, int _myPort) {
	initializeSocket(_myAddress, _myPort);
}

UDPSocket::UDPSocket(std::string _myAddress, int _myPort, std::string _peerAddress, int _peerPort) {
	initializeSocket(_myAddress, _myPort);
	setPeerAddress(_peerAddress, _peerPort);
}

UDPSocket::UDPSocket() {
	initializeSocket("0.0.0.0", 0);
}


//negative return values indicate failures
bool UDPSocket::write(std::vector<uint8> buffer){
	int msgLength = buffer.size();

	bool successFlag = true;

	if (msgLength > MESSAGE_LENGTH){
		std::cerr << "Message size should be less than " << MESSAGE_LENGTH << " bytes\n";
		successFlag = false;
	} else{ //attempt to send
		int number_bytes_sent = sendto(sock, &buffer[0], msgLength, 0, peerAddr_cast, sizeof(struct sockaddr));
		if (number_bytes_sent < 0){
			successFlag = false;
			switch(errno){
				case EMSGSIZE:
					std::cerr << "Message is too large to be sent atomically\n";
					break;
					//more errors here as needed
			}
		} else if (number_bytes_sent < msgLength){
			successFlag = false;
			std::cerr << "Not all bytes were sent (a full sending buffer?)\n";
		}
	}

	return successFlag;
}

void UDPSocket::setReadTimeoutDuration(int timeout_s, int timeout_ms){
	struct timeval timeout_dur;
	timeout_dur.tv_sec = timeout_s;
	timeout_dur.tv_usec = timeout_ms*1000;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_dur, sizeof(timeout_dur));
}

std::vector<uint8> UDPSocket::read(std::string *newPeerIP, int *newPeerPort){
	char *buffer = new char[MESSAGE_LENGTH];

	sockaddr_in newPeerAddr;
	sockaddr *newPeerAddr_cast = (sockaddr *)&newPeerAddr;
	socklen_t leng = sizeof(newPeerAddr_cast);
	int number_bytes_read = recvfrom(sock, buffer, MESSAGE_LENGTH, 0, newPeerAddr_cast, &leng);
	if (number_bytes_read < 0){ //==0??
		switch (errno){
			case ETIMEDOUT:
			case EAGAIN:
				std::cerr << "Reading timed out\n";
				break;
			case ENOTSOCK:
				std::cerr << "The socket arg does not refer to a socket\n";
				break;
			case EIO:
				std::cerr << "IO error\n";
				break;
			case ENOBUFS :
				std::cerr << "Insufficient resources to receive\n";
				break;
			default:
				std::cerr << "Check errnos of recvfrom\n" << "code: " << errno;
				//std::cerr << '\n' << EINVAL << " " << ENOMEM << " " << EFAULT <<  " " << ECONNREFUSED << " " << EAGAIN << " " << EWOULDBLOCK <<  '\n';
		}
		std::vector<uint8> empty_vector(0);
		return empty_vector; //??
	}

	std::vector<uint8> ret_vector(buffer, buffer+number_bytes_read);
	delete [] buffer;
	
	newPeerIP->assign(inet_ntoa(newPeerAddr.sin_addr));
	*newPeerPort = ntohs(newPeerAddr.sin_port);
	return ret_vector;
}

int UDPSocket::getMyPort() {
    return myPort;
}

int UDPSocket::getPeerPort() {
    return peerPort;
}


UDPSocket::~UDPSocket() {
    //pthread_mutex_unlock(&mutex);
    if (sock > 0) {
        close(sock);
    }
}
