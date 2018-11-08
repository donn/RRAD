#ifndef _request_generator_h
#define _request_generator_h

#include "Message.h"

#include <string>

namespace RRAD {
    class RequestGenerator {
        std::string userName;
        uint64 requestCounter = 0;
    public:
        RequestGenerator(std::string userName): userName(userName) {}
        Message requestList(std::string className, std::string targetUser);
    };
}

#endif // _request_generator_h