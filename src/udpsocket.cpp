// Project
#include "udpsocket.hpp"
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
    if (sock < 0) {
        throw errno;
    }

    std::cout << "Binding..." << std::endl;
    if (bind(sock, myAddr_cast, sizeof(struct sockaddr)) < 0) {
        std::cerr << "Failed to bind." << std::endl;
        throw errno;
    }
    std::cout << "Bound to receive from" << myAddress << ":" << myPort << std::endl;
}

UDPSocket::UDPSocket(std::string _myAddress, int _myPort) {
	initializeSocket(_myAddress, _myPort);
}

UDPSocket::UDPSocket() {
	initializeSocket("0.0.0.0", 0);
}

/*

bool UDPSocket::initializeClient(char * _peerAddr, int _peerPort) {

    peerPort = _peerPort;

    peerAddress = new char[strlen(_peerAddr) + 1];
    strcpy(peerAddress, _peerAddr);

    myAddr.sin_family = AF_INET; // inet_addr(_myAddr);
    myAddr.sin_port = htons(0);
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    std::cout << "Connecting..." << std::endl;
    if (bind(sock, myAddr_cast, sizeof(struct sockaddr)) < 0) {
        return false;
    }

    peerAddr.sin_family = AF_INET;
    peerAddr.sin_port = htons(_peerPort);
    inet_aton(_peerAddr,&peerAddr.sin_addr);
	return true;
}
*/


char *UDPSocket::getFilterAddress() {
    return NULL;
}

void UDPSocket::setFilterAddress (char * _filterAddress) {
    
}

//negative return values indicate failures
int UDPSocket::writeToSocket(char * buffer,  int maxBytes ) {
	if (maxBytes > MESSAGE_LENGTH){
		std::cerr << "Message size should be less than " << MESSAGE_LENGTH << " bytes\n";
		return -1;
	}
	int ret_val = sendto(sock, buffer, maxBytes, 0, peerAddr_cast, sizeof(struct sockaddr));
	if (ret_val < 0){
		switch(errno){
			case EMSGSIZE:
			std::cerr << "Message is too large to be sent atomically\n";
			break;
			//more errors here as needed
		}
	}

	return ret_val;
}

int UDPSocket::writeToSocketAndWait(char * buffer, int  maxBytes,int microSec ) {
	if (maxBytes > MESSAGE_LENGTH){
		std::cerr << "Message size should be less than " << MESSAGE_LENGTH << " bytes\n";
		return -1;
	}
	int ret_val = writeToSocket(buffer, maxBytes);
	usleep(microSec);

    return ret_val;
}

int UDPSocket::readFromSocketBasic(char * buffer,  int maxBytes ) {
	int leng = sizeof(sockaddr_in);
	int ret_val = recvfrom(sock, buffer, maxBytes, 0, peerAddr_cast, (socklen_t *)&leng);
	if (ret_val < 0){
		switch (errno){
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
	}
	return ret_val;
}

int UDPSocket::readFromSocketWithNoBlock(char * buffer, int  maxBytes ) {
    return -1;
}

int UDPSocket::readFromSocketWithTimeout(char * buffer, int maxBytes, int timeoutSec, int timeoutMilli) {
	struct timeval timeout_dur;
	timeout_dur.tv_sec = timeoutSec;
	timeout_dur.tv_usec = timeoutMilli*1000;

	//change socket operation
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_dur, sizeof(timeout_dur));
	int leng = sizeof(sockaddr_in);
	auto ret_val = readFromSocketBasic(buffer, maxBytes);
	timeout_dur.tv_sec = timeout_dur.tv_usec = 0;

	//reset socket operation
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_dur, sizeof(timeout_dur));
	return ret_val;
}

//size of receiced data
int UDPSocket::readFromSocketWithBlock(char * buffer,  int maxBytes ) {
	return readFromSocketBasic(buffer, maxBytes);
}

int UDPSocket::readSocketWithNoBlock(char * buffer, int  maxBytes ) {
    return -1;
}

int UDPSocket::readSocketWithTimeout(char * buffer, int maxBytes, int timeoutSec, int timeoutMilli) {
    return -1;
}

int UDPSocket::readSocketWithBlock(char * buffer,  int maxBytes) {
    return -1;
}

int UDPSocket::getMyPort() {
    return myPort;
}

int UDPSocket::getPeerPort() {
    return peerPort;
}

void UDPSocket::enable() {
    enabled = true;
}

void UDPSocket::disable() {
    enabled = false;
}

bool UDPSocket::isEnabled() {
    return enabled;
}

void UDPSocket::lock() {
    pthread_mutex_lock(&mutex);
}

void UDPSocket::unlock() {
    pthread_mutex_unlock(&mutex);
}

int UDPSocket::getSocketHandler() {
    return sock;
}

UDPSocket::~UDPSocket() {
    //pthread_mutex_unlock(&mutex);
    if (sock > 0) {
        close(sock);
    }
}
