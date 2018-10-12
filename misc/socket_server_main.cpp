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
	UDPSocket udp("0.0.0.0", 3333);
	std::string ip;
	int port;
	std::vector<uint8> vi = udp.read(&ip, &port);
	
	std::cout << "received " << vi.size() << '\n';

	if (!vi.empty())
		std::cout << (int)vi[0];

	return 0;
}
