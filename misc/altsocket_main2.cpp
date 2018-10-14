// c++ -std=c++17 -I include -D_INCREDIBLY_STUPID_ALTSOCKET_TESTING misc/altsocket_main.cpp src/Connection.cpp src/Packet.cpp src/UDPAltSocket.cpp -o test.out && lldb ./test.out
#include "Connection.h"
using namespace RRAD;

#include <iostream>

int main() {
    Connection c("0.0.0.0", 9001, 0, 0);
	std::string s = "have fun";
	c.write ({});

	//c.write ({'h', 'a', 'v', 'e', ' ', 'f', 'u', 'n'});

    c.listen([&](Connection cn){
        auto blob = cn.read();
        std::string str(blob.begin(),blob.end());
        std::cout << str << std::endl;

        cn.write({'l', 'o', 'l'});
    });

}
