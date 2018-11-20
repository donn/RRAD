#include "Dispatcher.h"

#include <iostream>

RRAD::Dispatcher::Dispatcher(std::string userName, uint16 port, bool forwardingEnabled) {
    this->userName = userName;
    port = port;
    connection = RRAD::Connection("0.0.0.0", 0, 0, port);
    std::cout << "[RRAD] Dispatcher connection opened on port " << port << "." << std::endl;

    this->forwardingEnabled = forwardingEnabled;
}

void RRAD::Dispatcher::setUID(std::string newUID) {
    static int changedOnce = 0;
    if (userName != newUID) {
        if (changedOnce++) {
            throw "dispatcher.userNameAlreadySet";
        }
        userName = newUID;
    }
}

void RRAD::Dispatcher::forwardRequests(std::string userName, std::string ip, uint16 port) {
    std::thread thread = std::thread([&]() {
        auto cn = RRAD::Connection(ip, port);
        auto& forwardQueue = forwardQueues[userName];
        try {
            while (!forwardQueue.empty()) {
                auto forwardable = forwardQueue.front();
                cn.write(forwardable.marshall());
            }
        } catch (const char* error) {
            std::cerr << "[RRAD] Unable to forward some messages to " << userName << " on " << ip << ":" << port << ", retrying on next authentication." << std::endl;
        }
    });
    thread.detach();
}

std::optional<RRAD::Message> RRAD::Dispatcher::doOperation(Message message) {
    // The forwarding fiasco
    auto recipient = message.msg_json["receiverID"];
    if (recipient != userName) {
        if (forwardingEnabled) {
            if (forwardQueues.find(recipient) != forwardQueues.end()) {
                forwardQueues[recipient] = std::queue<RRAD::Message>();
            }
            forwardQueues[recipient].push(message);
            return message.generateReply({"cached", "cached"});
        } else {
            return message.generateReply({"error", "cacheingUnsupported"});
        }
    }

    // Request realm
    auto object = message.getObject();

    if (cm) {
        cm->verifyArguments(&message.msg_json["operation"]);
    }

    if (object["id"] == 0 && object["unixTimestamp"] == 0) {
        auto array = JSON::array();
        for (auto ro: dictionary) {
            auto& iterable = *ro.second.second;
            if (
                iterable.getClassName() == object["class"]
                && ro.second.first
            ) {
                array.push_back(JSON::parse(ro.first));
            }
        }
        return message.generateReply(array);
    }
    if (
        dictionary.find(object.dump()) == dictionary.end()
        || object["ownerID"] != userName
    ) {
        return message.generateReply({"error", "objectNotFound"});
    }

    auto& target = *dictionary[object.dump()].second;
    auto result = message.generateReply(target.executeRPC(message.getOperation(), message.getArguments()));
    if (message.getOperation().find("__") == 0) {
        return std::nullopt;
    }
    return result;
}

RRAD::RemoteObject* RRAD::Dispatcher::getObject(JSON id) {
    dictionaryMutex.lock();
    std::cerr << id << std::endl;
    auto string = id.dump();
    if (dictionary.find(string) == dictionary.end()) {
        return NULL;
    }
    auto ro = dictionary[id.dump()].second;
    dictionaryMutex.unlock();
    return ro;
}

void RRAD::Dispatcher::registerObject(JSON id, RemoteObject* registree, bool owned) {
    dictionaryMutex.lock();
    dictionary[id.dump()] = std::pair(owned, registree);
    dictionaryMutex.unlock();
}

void RRAD::Dispatcher::destroyObject(JSON id) {
    dictionaryMutex.lock();
    auto idString = id.dump();
    delete dictionary[idString].second;
    dictionary.erase(idString);
    dictionaryMutex.unlock();
}

void RRAD::Dispatcher::syncLoop() {
    connection.listen([&](Connection* cn) {
        Message request;
        try {
            request = Message::getRequest(cn);
            auto reply = doOperation(request);
            if (reply.has_value()) {
                cn->write(reply.value().marshall());
            }
        } catch (const char* err) {
            std::cerr << "[RRAD] Failed to honor request from " << cn->ip << ": " << err << ": Dump" << std::endl;
            if (request.msg_json["senderID"] != "string") {
                std::cout << request.msg_json.dump();
            }
            std::cout << std::endl << std::endl;
        }
    });
}

void RRAD::Dispatcher::start() {
    std::thread tr ([&]() {
        syncLoop();
    });
    tr.detach();
}

std::vector<RRAD::RemoteObject*> RRAD::Dispatcher::listMine(std::string className) {
    std::vector<RRAD::RemoteObject*> vec;
    for (auto ro: dictionary) {
        auto& iterable = *ro.second.second;
        if (
            iterable.getClassName() == className
            && ro.second.first
        ) {
            vec.push_back(ro.second.second);
        }
    }
    return vec;
}

RRAD::Message RRAD::Dispatcher::listRPC(std::string className, std::string targetUser) {
    auto message = RRAD::Message();
    message.msg_json["senderID"] = userName;
    message.msg_json["receiverID"] = targetUser;
    message.msg_json["requestID"] = requestCounter++;
    message.msg_json["object"]["ownerID"] = targetUser;
    message.msg_json["object"]["unixTimestamp"] = 0;
    message.msg_json["object"]["id"] = 0;
    message.msg_json["object"]["class"] = className;
    message.msg_json["operation"]["name"] = "__DEVE__LIST";
    message.msg_json["operation"]["data"] = {};
    return message;
}

RRAD::Message RRAD::Dispatcher::rmiReqMsg(std::string className, std::string targetUser, JSON id, std::string method, JSON arguments) {
    auto message = listRPC(className, targetUser);
    message.msg_json["object"] = id;
    message.msg_json["operation"]["name"] = method;
    message.msg_json["operation"]["data"] = arguments;
    return message;
}

JSON RRAD::Dispatcher::communicateRMI(std::string targetIP, uint16 port, RRAD::Message rmiReqMsg) {
    std::cout << targetIP << " " << port << std::endl;
    if (cm) {
        cm->encodeArguments(&rmiReqMsg.msg_json["operation"]);
    }
    auto conn = RRAD::Connection(targetIP, port);
    conn.write(rmiReqMsg.marshall());
    auto reply = RRAD::Message::unmarshall(conn.read());
    return reply.getArguments();
}
