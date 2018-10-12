// Project Headers
#include "udpsocket.hpp"
#include "common.hpp"

// C STD
#include <stdlib.h>
#include <string.h> 

// CPP STL
#include <iostream>

// POSIX?
#include <unistd.h>

int main(int argc , char *argv[]) {

	UDPSocket udp ("0.0.0.0", 0);
	udp.setPeerAddress("0.0.0.0", 3333);
	std::vector<uint8> vi;
	vi.push_back (15);
	vi.push_back (14);
	std::cout << "sending " << (int)vi[0] << '\n';
	udp.write(vi);

	return 0;
}
