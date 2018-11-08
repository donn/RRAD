#ifndef _request_generator_h
#define _request_generator_h

#include "Message.h"
#include "Connection.h"
#include <string>

namespace RRAD {
    class RequestGenerator {
        std::string userName;
        uint64 requestCounter = 0;
    public:
        RequestGenerator(std::string userName): userName(userName) {}
        Message listRPC(std::string className, std::string targetUser);
        Message rmi(std::string className, std::string targetUser, JSON id, std::string method, JSON arguments);
        JSON communicateRMI(std::string targetIP, uint16 targetPort, Message rmi);
        std::string getUserName() { return userName; }
    };
}

#endif // _request_generator_h