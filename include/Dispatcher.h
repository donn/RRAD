#ifndef _dispatcher_h
#define _dispatcher_h

#include "Types.h"
#include "Message.h"

namespace RRAD {
    class RemoteObject {
    public:
        virtual JSON executeRPC(std::string name, JSON arguments) = 0;
        virtual std::string getClassName() = 0;
    };

    class Dispatcher {
        std::mutex dictionaryMutex;
        std::map<std::string, RemoteObject*> dictionary;
        Connection connection;

        Message doOperation(Message message); 
    public:
        Dispatcher(uint16 port = 9000);
        void registerObject(JSON id, RemoteObject* registree);
        void destroyObject(JSON id);
        void syncLoop();
        void start();
    };
}

#endif // _dispatcher_h
