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

	RRAD::UDPSocket udp ("0.0.0.0", 0);
	udp.setPeerAddress("0.0.0.0", 3333);
	std::vector<uint8> vi;
	vi.push_back (15);
	vi.push_back (14);
	std::cout << "sending " << (int)vi[0] << '\n';
	try{
		udp.write(vi);
	} catch (std::string e){
		std::cout << e << std::endl;
	}


	return 0;
}
