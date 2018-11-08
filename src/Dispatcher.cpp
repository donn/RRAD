#include "Dispatcher.h"

#include <iostream>

RRAD::Dispatcher::Dispatcher(std::string userName, uint16 port, bool forwardingEnabled) {
    this->userName = userName;
    port = port;
    connection = RRAD::Connection("0.0.0.0", 0, 0, port);
    std::cout << "[RRAD] Dispatcher connection opened on port " << port << "." << std::endl;

    this->forwardingEnabled = forwardingEnabled;
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

    if (object["id"] == 0 && object["unixTimestamp"] == 0) {
        auto array = JSON::array();
        for (auto ro: dictionary) {
            auto& iterable = *ro.second;
            if (
                iterable.getClassName() == object["class"]
                && userName == object["ownerID"]
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

    auto& target = *dictionary[object.dump()];
    auto result = message.generateReply(target.executeRPC(message.getOperation(), message.getArguments()));
    if (message.getOperation().find("__") == 0) {
        return std::nullopt;
    }
    return result;
}

void RRAD::Dispatcher::registerObject(JSON id, RemoteObject* registree) {
    dictionaryMutex.lock();
    dictionary[id.dump()] = registree;
    dictionaryMutex.unlock();
}

void RRAD::Dispatcher::destroyObject(JSON id) {
    dictionaryMutex.lock();
    dictionary.erase(id.dump());
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
    std::thread([&]() {
        syncLoop();
    });
}