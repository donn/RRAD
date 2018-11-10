#ifndef _connection_h
#define _connection_h

#include <future>
#include <functional>

#include "Types.h"
#include "UDPSocket.h"

#define MAX_RETRIALS 5

namespace RRAD {
    class Connection {
        uint16 port;
        int timeout;
        std::shared_ptr<UDPSocket> socketp; //using a pointer to evade the default constructor
    public:
        std::string ip;

        Connection(std::string ip = "0.0.0.0", uint16 port = 0, int timeout = 1000, uint16 localPort = 0);
		~Connection();

        std::vector<uint8> read();
        void write(std::vector<uint8> data);
        void listen(std::function<void(Connection*)> operativeLoop);

        std::promise< std::vector<uint8> > getData();
        std::promise< bool > sendData(std::vector<uint8> data);
    };
}

#endif
