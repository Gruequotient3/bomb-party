#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <string>

class TCPSocketServer{
    public:
        int fdSocket;
        TCPSocketServer(const char* port);
        ~TCPSocketServer();

        int Accept(struct sockaddr* from, unsigned int* len);
        int GetData(char* buffer, unsigned int len, int fd);
        int SendData(const char* data, unsigned int len, int fd);
};

class TCPSocketClient{
    public:
        int fdSocket;

        TCPSocketClient(const char* ip, const char* port);
        ~TCPSocketClient();

        int Connect(const std::string& addr, unsigned short port);
        int GetData(char* buffer, unsigned int len);
        int SendData(const char* data, unsigned int len);
};

#endif