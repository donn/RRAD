#ifndef _dispatcher_h
#define _dispatcher_h

#include "Types.h"
#include "Message.h"

#include <queue>
#include <optional>

namespace RRAD {
    class RemoteObject {
    public:
        JSON listRequest();
        virtual JSON executeRPC(std::string name, JSON arguments) = 0;
        virtual std::string getClassName(){
            return typeid(*this).name();
        };
    };

    class Dispatcher {
        std::mutex dictionaryMutex;
        std::map<std::string, RemoteObject*> dictionary;
        std::map<std::string, std::queue<Message> > forwardQueues;

        Connection connection;
        uint16 port;

        std::string userName;
        bool forwardingEnabled;
        
        std::optional<RRAD::Message> doOperation(Message message); 
    public:
        Dispatcher(std::string userName, uint16 port = 9000, bool forwardingEnabled = false);

        void forwardRequests(std::string target, std::string ip, uint16 port);

        void registerObject(JSON id, RemoteObject* registree);
        void destroyObject(JSON id);
        
        void syncLoop();
        void start(); //async

        std::string getUID() { return userName; }

        static Dispatcher singleton;
    };
}

#endif // _dispatcher_h
