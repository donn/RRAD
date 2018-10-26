#ifndef _message_h
#define _message_h

#include "JSON.h"
#include "Connection.h"

#define JSON_NULL JSON::value_t::null

using JSON = nlohmann::json;

namespace RRAD {
    class Message {
        JSON msg_json;
public:
        Message();
        Message(std::string senderID, std::string senderPW, std::string receiverID, bool request,
        uint32 requestID, JSON object, std::string operationName, JSON data);
        Message(JSON msg_json);

        void setSenderID(std::string senderID);
        void setSenderPW(std::string senderPW);
        void setReceiverID(std::string receiverID);
        void setRequestID(uint32 requestID);
        void setRequest(bool request);

        void setObject(JSON object);
        void setObjectTimestamp(uint32 num);

        void setOperationName(std::string name);
        void setOperationData(JSON data);

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
