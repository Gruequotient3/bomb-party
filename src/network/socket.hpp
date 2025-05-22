#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <string>
#include <map>


class TCPSocketServer{
    public:
        int fdSocket;
        
        TCPSocketServer(const char* port);
        ~TCPSocketServer();

        int Accept(struct sockaddr* from, unsigned int* len) const;
        int GetData(char* buffer, unsigned int len, int fd) const;
        int SendData(const char* data, unsigned int len, int fd) const;
        void SendError(const std::string& errorCode, int fd) const;

};

class TCPSocketClient{
    public:
        int fdSocket;

        TCPSocketClient(const char* ip, const char* port);
        ~TCPSocketClient();

        int GetData(char* buffer, unsigned int len) const;
        int SendData(const char* data, unsigned int len) const;
};

#endif