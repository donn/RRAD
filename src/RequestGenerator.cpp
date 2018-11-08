#include "RequestGenerator.h"

#include "Dispatcher.h"

RRAD::Message RRAD::RequestGenerator::requestList(std::string className, std::string targetUser) {
    auto message = RRAD::Message();
    message.msg_json["senderID"] = userName;
    message.msg_json["receiverID"] = targetUser;
    message.msg_json["object"]["ownerID"] = targetUser;
    message.msg_json["object"]["unixTimestamp"] = 0;
    message.msg_json["object"]["id"] = 0;
    message.msg_json["object"]["class"] = className;
    return message;
}

RRAD::Message RRAD::RequestGenerator::requestRMI(std::string className, std::string targetUser, JSON id, std::string method, JSON arguments) {
    auto message = requestList(className, targetUser);
    message.msg_json["object"] = id;
    message.msg_json["operation"]["name"] = method;
    message.msg_json["operation"]["arguments"] = arguments;
    return message;
}