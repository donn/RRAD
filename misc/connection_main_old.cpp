// c++ -g -std=c++17 -I include misc/connection_main.cpp src/Connection.cpp src/Packet.cpp src/UDPSocket.cpp -o test.out && lldb ./test.out
#include "Connection.h"
#include "Vectorize.h"
using namespace RRAD;

#include <iostream>

int main() {
	Connection c("0.0.0.0", 0, 0, 9001);

	c.listen([&](Connection *cn) {
		std::cout << "Connection received, processing...\n";
		auto blob = cn->read();
		std::string str(blob.begin(), blob.end());
		std::cout << str << std::endl;

		cn->write(vectorize("Then die!"));
	});
	return 0;
}