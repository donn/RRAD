// c++ -g -std=c++17 -I include misc/connection_main2.cpp src/Connection.cpp src/Packet.cpp src/UDPSocket.cpp -o test2.out && lldb ./test2.out
#include "Connection.h"
#include "Vectorize.h"
using namespace RRAD;

#include <iostream>

int main() {
    Connection c("0.0.0.0", 9001);
	std::string s = "Ready to die?";

	c.write(vectorize(s));
    std::cout << devectorizeToString(c.read()) << std::endl;

}