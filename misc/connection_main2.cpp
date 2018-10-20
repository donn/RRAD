// c++ -g -std=c++17 -I include misc/connection_main2.cpp src/Connection.cpp src/Packet.cpp src/UDPSocket.cpp -o test2.out && lldb ./test2.out
#include "Connection.h"
using namespace RRAD;

#include <iostream>

int main() {
    Connection c("0.0.0.0", 9001, 0, 0);
	std::string s = "have fun";
	//c.write ({});

    //initialize connection and send a first msg
	c.write (std::vector<uint8>(s.begin(), s.end()));
    while(1){
        char transaction;
        std::string s;
        std::cout << "#####receive ('r') or not?";
        std::cin >> transaction; std::cin.ignore();

        std::cout << "#####";
        if (transaction == 'r'){
            auto blob = c.read();
            s = std::string(blob.begin(),blob.end());
            std::cout << "P2: " << s << std::endl;
        } else {
            std::cout << "P2: gonna send: ";
            std::getline(std::cin, s); std::cin.clear();
            c.write (std::vector<uint8>(s.begin(), s.end()));

        }
    }

}
