#include "RequestGenerator.h"

#include "Dispatcher.h"

RRAD::Message RRAD::RequestGenerator::requestList(std::string className, std::string targetUser) {
    auto message = RRAD::Message();
    message.msg_json["senderID"] = RRAD::Dispatcher::singleton.getUID();
    message.msg_json["receiverID"] = targetUser;
    message.msg_json["object"]["ownerID"] = targetUser;
    message.msg_json["object"]["unixTimestamp"] = 0;
    message.msg_json["object"]["id"] = 0;
    message.msg_json["object"]["class"] = className;
    return message;
}