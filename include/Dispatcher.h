#ifndef _dispatcher_h
#define _dispatcher_h

#include "Types.h"
#include "Message.h"

namespace RRAD {
    class RemoteObject {
    public:
        JSON listRequest();
        virtual JSON executeRPC(std::string name, JSON arguments) = 0;
        virtual std::string getClassName() = 0;
    };

    class Dispatcher {
        std::string userName;
        std::mutex dictionaryMutex;
        std::map<std::string, RemoteObject*> dictionary;
        Connection connection;

        Message doOperation(Message message); 
    public:
        Dispatcher(std::string userName, uint16 port = 9000);
        void registerObject(JSON id, RemoteObject* registree);
        void destroyObject(JSON id);
        void syncLoop();
        void start();
        std::string getUID() { return userName; }

        static Dispatcher singleton;
    };
}

#endif // _dispatcher_h
