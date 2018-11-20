// c++ -g -std=c++17 -I include misc/connection_main2.cpp src/Connection.cpp src/Packet.cpp src/UDPSocket.cpp -o test2.out && lldb ./test2.out
#include "Connection.h"
#include "Vectorize.h"
using namespace RRAD;

#include <iostream>

int main() {
    Connection c("0.0.0.0", 9001);
	std::string s = "Ready to die?";
    std::vector<uint8> vt(1009, 'H');
    std::vector<uint8> vh(40, 'A');
    vt.insert(vt.end(), vh.begin(), vh.end());

	c.write(vt);
    std::cout << devectorizeToString(c.read()) << std::endl;

}