#ifndef _message_h
#define _message_h

#include "JSON.h"
#include "Connection.h"

#define JSON_NULL JSON::value_t::null

using JSON = nlohmann::json;

namespace RRAD {
    class Message {
    public:
        JSON msg_json;
        Message();
        Message(JSON msg_json);

        //getters as needed
        bool isRequest();
        JSON getObject();
        std::string getOperation();
        JSON getArguments(); //json array

        //turning strings into bytes isn't marshalling but whatevs; just need to care about endianness
        std::vector<uint8> marshall();
        static Message unmarshall(std::vector<uint8> bytes);

        static Message getRequest(Connection* connection); // Read and unmarshal JSON from connection
        Message generateReply(JSON returnData);
    };
}

#endif // _message_h
