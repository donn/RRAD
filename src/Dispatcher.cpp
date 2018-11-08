#include "Dispatcher.h"

#include <iostream>

RRAD::Dispatcher::Dispatcher(std::string userName, uint16 port) {
    this->userName = userName;
    port = port;
    connection = RRAD::Connection("0.0.0.0", 0, 0, port);
    std::cout << "[RRAD] Dispatcher connection opened on port " << port << "." << std::endl;
}

RRAD::Message RRAD::Dispatcher::doOperation(Message message) {
    auto object = message.getObject();
    if (object["id"] == 0 && object["unixTimestamp"] == 0) {
        auto array = JSON::array();
        for (auto ro: dictionary) {
            auto& iterable = *ro.second;
            if (iterable.getClassName() == object["class"]) {
                array.push_back(iterable.executeRPC(message.getOperation(), message.getArguments()));
            }
        }
        return message.generateReply(array);
    }
    if (dictionary.find(object.dump()) == dictionary.end()) {
        return message.generateReply({"error", "objectNotFound"});
    }

    auto& target = *dictionary[object.dump()];
    return message.generateReply(target.executeRPC(message.getOperation(), message.getArguments()));
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
        try {
            auto request = Message::getRequest(cn);
            auto reply = doOperation(request);
            cn->write(reply.marshall());
        } catch (const char* err) {
            std::cout << "Failed to honor request from " << cn->ip << ": " << err << std::endl;
        }
    });
}

void RRAD::Dispatcher::start() {
    std::thread([&]() {
        syncLoop();
    });
}