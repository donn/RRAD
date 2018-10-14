#ifndef _connection_h
#define _connection_h

#include <future>
#include <functional>

#include "Types.h"
#include "UDPSocket.h"

namespace RRAD {
    class Connection {
        std::string ip;
        uint16 port;
        int timeout;
        UDPSocket *socketp; //using a pointer to evade the default constructor
    public:
        Connection(std::string ip, uint16 port, int timeout = 1000, uint16 localPort = 0);
		~Connection();

        std::vector<uint8> read();
        void write(std::vector<uint8> data);
        void listen(std::function<void(Connection&)> operativeLoop);

        std::promise< std::vector<uint8> > getData();
        std::promise< bool > sendData(std::vector<uint8> data);
    };
}

#endif
