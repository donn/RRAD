#ifndef _udpsocket_h
#define _udpsocket_h


#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string>
#include <vector>

#define MESSAGE_LENGTH	1024
#define BUFFER_SIZE 	8

namespace RRAD {

    class UDPSocket {
    protected:
        int sock;

        //"my" address/port refers to the address/port the socket can receive from
        //for a server: 0.0.0.0 accepts connections from all interfaces
        std::string myAddress;
        int myPort;
        sockaddr_in myAddr;
        sockaddr* myAddr_cast;
        
        std::string peerAddress;
        int peerPort;
        sockaddr_in peerAddr;
        sockaddr* peerAddr_cast;

        bool enabled;

        pthread_mutex_t mutex;

        void initializeSocket(std::string _myAddress, int _myPort);
        int readFromSocketBasic(char * buffer,  int maxBytes );

    public:
        //by default ip = "0.0.0.0", receivingPort = 0
        UDPSocket(std::string _myAddress, int _myPort);
        UDPSocket();

        void setFilterAddress(char * _filterAddress);
        char *getFilterAddress();
        bool initializeServer(char * _myAddr, int _myPort);
        bool initializeClient(char * _peerAddr, int _peerPort);
        int writeToSocket(char * buffer,  int maxBytes );
        int writeToSocketAndWait(char * buffer, int  maxBytes,int microSec );
        int readFromSocketWithNoBlock(char * buffer, int  maxBytes );
        int readFromSocketWithTimeout(char * buffer, int maxBytes, int timeoutSec, int timeoutMilli);
        int readFromSocketWithBlock(char * buffer,  int maxBytes );
        int readSocketWithNoBlock(char * buffer, int  maxBytes );
        int readSocketWithTimeout(char * buffer, int maxBytes, int timeoutSec, int timeoutMilli);
        int readSocketWithBlock(char * buffer,  int maxBytes );
        int getMyPort();
        int getPeerPort();
        void enable();
        void disable();
        bool isEnabled();
        void lock();
        void unlock();
        int getSocketHandler();
        ~UDPSocket ( );
    };
}

#endif // _udpsocket_h