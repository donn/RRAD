// c++ -std=c++17 -I include -D_INCREDIBLY_STUPID_ALTSOCKET_TESTING misc/test.cpp src/Connection.cpp src/Packet.cpp src/UDPAltSocket.cpp -o test.out && lldb ./test.out
#include "Connection.h"
using namespace RRAD;

#include <iostream>

int main() {
    Connection c("0.0.0.0", 41, 1000);

    c.listen([&](Connection cn){
        auto blob = cn.read();
        std::string str(blob.begin(),blob.end());
        std::cout << str << std::endl;

        cn.write({'w', 'a', 'k', 'e', 'm', 'e', 'u', 'p'});
    });

}