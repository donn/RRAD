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
        UDPSocket socket;
    public:
        Connection(std::string ip, uint16 port, int timeout = 1000);

        void listen(std::function<void(Connection, std::vector<uint8>)> operativeLoop);

        std::promise< std::vector<uint8> > getData();
        std::promise< bool > sendData(std::vector<uint8> data);
    };
}

#endif