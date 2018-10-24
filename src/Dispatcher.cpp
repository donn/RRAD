#if 0
#include "Dispatcher.h"

RRAD::Dispatcher::Dispatcher(uint16 port) {
    connection = RRAD::Connection("0.0.0.0", 0, 1000, port);
}

RRAD::Message RRAD::Dispatcher::getRequest() {
    return RRAD::Message::getRequest(&connection);
}

RRAD::Message RRAD::Dispatcher::doOperation(Message message) {
    auto object = message.getID();
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
#endif