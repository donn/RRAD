// Project Headers
#include "UDPSocket.h"

// C STD
#include <stdlib.h>
#include <string.h> 

// CPP STL
#include <iostream>

// POSIX?
#include <unistd.h>

int main(int argc , char *argv[]) {
	RRAD::UDPSocket udp("0.0.0.0", 3333, 3333);
	std::string ip;
	unsigned short port;
	std::vector<uint8> vi = udp.read(&ip, &port);	
	std::cout << "received " << vi.size() << " bytes" << std::endl;

	if (!vi.empty())
		std::cout << (int)vi[0];

	return 0;
}
