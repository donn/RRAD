// c++ -g -std=c++17 -I include misc/connection_main.cpp src/Connection.cpp src/Packet.cpp src/UDPSocket.cpp -o test.out && lldb ./test.out
#include "Connection.h"
using namespace RRAD;

#include <iostream>

int main() {
	Connection c("0.0.0.0", 44411, 0, 9001);

	c.listen([&](Connection &cn){
		auto blob = cn.read();
		char transaction;
		std::string s;
		s = std::string(blob.begin(),blob.end());
		std::cout << "connection received\n" << " Got " << s << " as first thing\n";
		while(1){
			std::cout << "receive ('r') or not?";
			std::cin >> transaction; std::cin.ignore();

			std::cout << "#####";
			if (transaction == 'r'){
				blob = cn.read();
				s = std::string(blob.begin(),blob.end());
				std::cout << "P1: got " << s << std::endl;
			} else {
				s = "Last time got " + s;
				cn.write (std::vector<uint8>(s.begin(), s.end()));
				std::cout << "P1: gonna send: " << s << std::endl;
			}
		}
	});
	return 0;
}
