#include <ctime>
#include <iostream>
#include <string>
#include "Message.h"

JSON req_msg_format = R"(
{
    "senderID": "string",
    "senderPW": "string",
    "receiverID": "string",
    "requestID": 1,
    "request": true,
    "object": {
        "ownerID": "string",
        "unixTimestamp": 1,
        "id": 1,
        "class": "string"
    },
    "operation": {
        "name": "string",
        "data": {}
    }
}
)"_json;

//check the candidate_content format against the message format (depth = 1)
//remove later prolly
bool isValidFormat(JSON candidate_content){
    for (auto it : req_msg_format.items()) {
        if (candidate_content.count(it.key()) == 1){
            if (candidate_content[it.key()].type() != it.value().type())
                return false;
        } else
            return false;
    }

    return true;
}

RRAD::Message::Message(){
    *this = Message(req_msg_format);
}
RRAD::Message::Message(JSON msg_json){
    if(!isValidFormat(msg_json))
        throw "Invalid.Msg.JSON";
    this->msg_json = msg_json;
}
std::vector<uint8> RRAD::Message::marshall(){
    std::string serialized_content = msg_json.dump();
    std::vector<uint8> marshalled_bytes(serialized_content.begin(), serialized_content.end());
    return marshalled_bytes;
}

RRAD::Message RRAD::Message::unmarshall(std::vector<uint8> bytes){
    JSON msg_json;

    try {
        msg_json = JSON::parse(bytes);
    } catch (JSON::parse_error& e){
        std::cerr << "Error: " << e.what() << "\n"
                << "byte position of error: " << e.byte << std::endl;
    }

    return Message(msg_json);
}

JSON RRAD::Message::getObject(){
    return msg_json["object"];
}

std::string RRAD::Message::getOperation(){
    return msg_json["operation"]["name"];
}

JSON RRAD::Message::getArguments(){
    return msg_json["operation"]["data"];
}

// Read and unmarshal JSON from connection
RRAD::Message RRAD::Message::getRequest(Connection *connection){
    std::vector<uint8> message_bytes = connection->read();
    Message request = unmarshall(message_bytes);
    if (!request.msg_json["request"]) {
        throw "message.notRequest";
    }
    request.msg_json["operation"]["data"]["__RRAD__INTERNAL__senderIP"] = connection->ip; // Hack lmao
    return request;
}

RRAD::Message RRAD::Message::generateReply(JSON returnData){
    //consider a later size optimization by removing unncessary fields
    Message reply(msg_json);
    reply.msg_json["reply"] = false;
    reply.msg_json["operation"]["data"] = returnData;

    return reply;
}