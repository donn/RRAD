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
        virtual std::string getClassName() {
            return typeid(*this).name();
        };
    };

    class CryptoModule {
    public:
        virtual void encodeArguments(JSON* json) = 0;
        virtual void verifyArguments(JSON* json) = 0;
    };

    class Dispatcher {
        std::mutex dictionaryMutex;
        std::map<std::string, std::pair<bool, RemoteObject*> >dictionary;
        std::map<std::string, std::queue<Message> > forwardQueues;

        Connection connection;
        uint16 port;

        std::string userName;
        bool forwardingEnabled;
        
        uint64 requestCounter = 0;
        
        std::optional<RRAD::Message> doOperation(Message message); 
    public:
        Dispatcher(std::string userName, uint16 port = 20000, bool forwardingEnabled = false);

        CryptoModule* cm;
    
        void forwardRequests(std::string target, std::string ip, uint16 port);

        // Object Registration
        void registerObject(JSON id, RemoteObject* registree, bool owned = true);
        void destroyObject(JSON id);
        
        // Start Listeners
        void syncLoop(); //sync
        void start(); //async

        // Owner data methods
        std::vector<RemoteObject*> listMine(std::string className);
        Message listRPC(std::string className, std::string targetUser);
        Message rmi(std::string className, std::string targetUser, JSON id, std::string method, JSON arguments);
        JSON communicateRMI(std::string targetIP, uint16 targetPort, Message rmi);

        void setUID(std::string newUID);
        std::string getUID() { return userName; }

        static Dispatcher singleton;
    };
}

#endif // _dispatcher_h
