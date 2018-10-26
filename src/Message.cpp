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

RRAD::Message::Message(std::string senderID, std::string senderPW, std::string receiverID, bool request,
        uint32 requestID, JSON object, std::string operationName, JSON data){
    setSenderID(senderID);
    setSenderPW(senderPW);
    setReceiverID(receiverID);
    setRequestID(requestID);
    setRequest(request);

    setObject(object);
    
    setOperationName(operationName);
    setOperationData(data);
}

RRAD::Message::Message(JSON msg_json){
    if(!isValidFormat(msg_json))
        throw "Invalid.Msg.JSON";
    this->msg_json = msg_json;
}

//setters
void RRAD::Message::setSenderID(std::string senderID){
    msg_json["senderID"] = senderID;
}

void RRAD::Message::setSenderPW(std::string senderPW){
    msg_json["senderPW"] = senderPW;
}

void RRAD::Message::setReceiverID(std::string receiverID){
    msg_json["receiverID"] = receiverID;
}

void RRAD::Message::setRequestID(uint32 requestID){
    msg_json["requestID"] = requestID;
}

void RRAD::Message::setRequest(bool request){
    msg_json["request"] = request;
}

void RRAD::Message::setObject(JSON object){
    object["unixTimestamp"] = time(nullptr);
    msg_json["object"] = object;
}

void RRAD::Message::setObjectTimestamp(uint32 num){
    msg_json["unixTimestamp"] = num;
}

void RRAD::Message::setOperationName(std::string name){
    msg_json["operation"]["name"] = name;
}

void RRAD::Message::setOperationData(JSON data){
    msg_json["operation"]["data"] = data;
}

//getters
bool RRAD::Message::isRequest(){
    return msg_json["request"];
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
        std::cerr << "Error: " << e.what() << '\n'
                << "byte position of error: " << e.byte << std::endl;
    }

    return Message(msg_json);
}

// Read and unmarshal JSON from connection
RRAD::Message RRAD::Message::getRequest(Connection *connection){
    std::vector<uint8> message_bytes = connection->read();
    Message request = unmarshall(message_bytes);
    if (!request.isRequest())
        throw "Invalid.Request";
    return request;
}

RRAD::Message RRAD::Message::generateReply(JSON returnData){
    //consider a later size optimization by removing unncessary fields
    Message reply(msg_json);
    reply.setRequest(false);
    reply.setOperationData(returnData);

    return reply;
}