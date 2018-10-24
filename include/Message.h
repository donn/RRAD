#ifndef _message_h
#define _message_h

#include "JSON.h"
#include "Connection.h"

using JSON = nlohmann::json;

namespace RRAD {
    class Message {
    public:
        static Message getRequest(Connection* connection); // Read and unmarshal JSON from connection
        Message generateReply(JSON returnData);
        JSON getID();
        std::string getOperation();
        JSON getArguments(); //json array
    };
};

#endif // _message_h