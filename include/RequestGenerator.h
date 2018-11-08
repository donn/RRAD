#ifndef _request_generator_h
#define _request_generator_h

#include "Message.h"

#include <string>

namespace RRAD {
    class RequestGenerator {
        uint64 requestCounter = 0;
    public:
        Message requestList(std::string className, std::string targetUser);
    };
}

#endif // _request_generator_h