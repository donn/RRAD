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
    public:
        Dispatcher(uint16 port = 9000);
        Message getRequest();
        Message doOperation(Message message); 
        void registerObject(JSON id, RemoteObject* registree);
        void destroyObject(JSON id);
        void sendReply();
    };
};

#endif // _dispatcher_h