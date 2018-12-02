#include "Dispatcher.h"

#include <iostream>

#ifndef _VERBOSE_DISPATCHER
#define _VERBOSE_DISPATCHER 0
#endif

RRAD::Dispatcher::Dispatcher(std::string userName, uint16 port, bool forwardingEnabled) {
    this->userName = userName;
    port = port;
    connection = RRAD::Connection("0.0.0.0", 0, 0, port);
    std::cout << "[RRAD] Dispatcher connection opened on port " << port << "." << std::endl;

    this->forwardingEnabled = forwardingEnabled;
}

void RRAD::Dispatcher::setUID(std::string newUID) {
    if (userName != newUID) {
        if (usernameSet++) {
            throw "dispatcher.userNameAlreadySet";
        }
        userName = newUID;
    }
}

void RRAD::Dispatcher::setForwarder(std::string newForwarderIP, uint16 newForwarderPort) {
    if (
        (forwarderIP.has_value() && forwarderIP.value() != newForwarderIP)
        || (newForwarderPort != forwarderPort)
    ) {
        if (forwarderSet++) {
            throw "dispatcher.forwarderAlreadySet";
        }
        forwarderIP = newForwarderIP;
        forwarderPort = newForwarderPort;
    }
}


void RRAD::Dispatcher::forwardRequests(std::string userName, std::string ip) {
    std::thread thread = std::thread([&]() {
        if (forwardQueues.find(userName) != forwardQueues.end()) {
            auto& forwardQueue = forwardQueues[userName];
            try {
                while (!forwardQueue.empty()) {
                    auto forwardable = forwardQueue.front();

                    auto conn = RRAD::Connection(ip, forwardable.msg_json["operation"]["data"]["RRAD::targetPort"]);

                    conn.write(forwardable.marshall());
                    conn.read();
                }
            } catch (const char* error) {
                std::cerr << "[RRAD] Unable to forward some messages to " << userName << " on " << ip << ":" << port << ", retrying on next authentication." << std::endl;
            }
        }
    });
    thread.detach();
}

RRAD::Message RRAD::Dispatcher::doOperation(Message message) {
    // The forwarding fiasco
    // If the current request is a forward request
    auto recipient = message.msg_json["receiverID"];
    auto sender = message.msg_json["senderID"];
    if (recipient != userName) {
        if (forwardingEnabled) {
            if (forwardQueues.find(recipient) == forwardQueues.end()) {
                forwardQueues[recipient] = std::queue<RRAD::Message>();
            }
            forwardQueues[recipient].push(message);
            #if _VERBOSE_DISPATCHER
            std::cout << "[RRAD] Cached message for " << recipient << std::endl;
            #endif
            return message.generateReply({"cached", "cached"});
        } else {
            return message.generateReply({"error", "cacheingUnsupported"});
        }
    }

    // Forward requests for a communication (asynchronous)
    forwardRequests(message.msg_json["senderID"], message.msg_json["operation"]["data"]["RRAD::senderIP"]);

    // Request realm
    auto object = message.getObject();

    if (cm) {
        auto operation = message.msg_json["operation"];

        for (auto it = operation["data"].begin(); it != operation["data"].end(); ++it) {
            if (it.key().find("RRAD::") == 0) {
                operation["data"].erase(it.key());
            }
        }

        cm->verifyArguments(sender, &operation);
    }

    if (object["id"] == 0 && object["unixTimestamp"] == 0) {
        auto mine = listMine(object["class"]);
        auto array = JSON::array();

        std::for_each(mine.begin(), mine.end(), [&](RemoteObject* ro){
                auto& obj = *ro;
                array.push_back(obj.getID());
        });
        return message.generateReply(array);
    }
    if (
        dictionary.find(object.dump()) == dictionary.end()
        || (object["ownerID"] != userName && object["ownerID"] != sender)
        // Q: allowing the sender to play with the objects he sent (setAccess)
        // this should be refined
        // A: Do not see the need to refine. This is intended behavior.
    ) {
        return message.generateReply({"error", "objectNotFound"});
    }

    auto& target = *dictionary[object.dump()].second;
    
    auto result = message.generateReply(target.executeRPC(message.getOperation(), message.getArguments()));

    // N: let the objects themselves manage that
    // NB: i mean... sure
    /*
    if (message.getOperation().find("__") == 0) {
        return std::nullopt;
    }
    */
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

void RRAD::Dispatcher::registerObject(RemoteObject* registree, bool owned) {
    dictionaryMutex.lock();
    //the below works because of the internal std::map representation of nlohmann JSONs
    dictionary[(registree->getID()).dump()] = std::pair(owned, registree);
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

            #if _VERBOSE_DISPATCHER
            std::cout << "[RRAD] Got request: " << request.msg_json.dump(4) << std::endl << std::endl;
            #endif

            request.msg_json["operation"]["data"]["RRAD::senderIP"] = cn->ip;
            request.msg_json["operation"]["data"]["RRAD::senderUserName"] = request.msg_json["senderID"];

            auto reply = doOperation(request);
            
            #if _VERBOSE_DISPATCHER
            std::cout << "[RRAD] Sending reply: " << reply.msg_json.dump(4) << std::endl << std::endl;
            #endif
            cn->write(reply.marshall());

        } catch (const char* err) {
            std::cerr << "[RRAD] Failed to honor request from " << cn->ip << ": " << err << ": Dump" << std::endl;
            if (request.msg_json["senderID"] != "string") {
                std::cout << request.msg_json.dump(4);
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
    message.msg_json["request"] = true;
    message.msg_json["senderID"] = userName;
    message.msg_json["receiverID"] = targetUser;
    message.msg_json["requestID"] = requestCounter++;
    message.msg_json["object"]["ownerID"] = targetUser;
    message.msg_json["object"]["unixTimestamp"] = 0;
    message.msg_json["object"]["id"] = 0;
    message.msg_json["object"]["class"] = className;
    message.msg_json["operation"]["name"] = "__DEVE__LIST";
    message.msg_json["operation"]["data"] = EmptyJSO;
    return message;
}

RRAD::Message RRAD::Dispatcher::rmiReqMsg(std::string className, std::string targetUser, JSON id, std::string method, JSON arguments) {
    auto message = listRPC(className, targetUser);
    message.msg_json["object"] = id;
    message.msg_json["operation"]["name"] = method;
    if (arguments.is_null()) {
        message.msg_json["operation"]["data"] = EmptyJSO;
    } else {
        message.msg_json["operation"]["data"] = arguments;
    }
    return message;
}

JSON RRAD::Dispatcher::communicateRMI(std::string targetIP, uint16 port, RRAD::Message rmiReqMsg) {
    
    if (cm) {
        cm->encodeArguments(&rmiReqMsg.msg_json["operation"]);
    }
    #if _VERBOSE_DISPATCHER
    std::cout << "[RRAD] Sending request: " << rmiReqMsg.msg_json.dump(4) << std::endl << std::endl;
    #endif

    rmiReqMsg.msg_json["operation"]["data"]["RRAD::targetIP"] = targetIP; 
    rmiReqMsg.msg_json["operation"]["data"]["RRAD::targetPort"] = port; 

    auto conn = RRAD::Connection(targetIP, port);
    auto marshalled = rmiReqMsg.marshall();

    std::cout << forwarderIP.has_value() << " : " << rmiReqMsg.getOperation() << std::endl;

    auto forward = false;
    try {
        conn.write(marshalled);
    } catch (const char *err) {
        if (
            forwarderIP.has_value()
            && (rmiReqMsg.getOperation().find("__") == 0)
        ) {
            forward = true;
        }
    }

    Message reply;

    if (forward) {
        auto conn = RRAD::Connection(forwarderIP.value(), forwarderPort);
        conn.write(marshalled);
        
        reply = RRAD::Message::unmarshall(conn.read());

        #if _VERBOSE_DISPATCHER
        std::cout << "[RRAD] Request forwarded. " << std::endl << std::endl;
        #endif
    } else {
        
        reply = RRAD::Message::unmarshall(conn.read()); 

        #if _VERBOSE_DISPATCHER
        std::cout << "[RRAD] Received reply: " << reply.msg_json.dump(4) << std::endl << std::endl;
        #endif
    }

    return reply.getArguments();
}
