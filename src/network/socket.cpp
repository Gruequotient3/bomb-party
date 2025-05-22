#ifndef SOCKET_CPP
#define SOCKET_CPP

#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "socket.hpp"

/*
|******************************|
|             Server           |
|******************************|
*/
TCPSocketServer::TCPSocketServer(const char* port){
    struct addrinfo hints, *gai, *ai;
    int err;
    int yes = 1;

    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((err = getaddrinfo(NULL, port, &hints, &gai))){
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }
    
    for (ai = gai; ai != NULL; ai = ai->ai_next){
        if ((fdSocket = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) < 0){
            perror("socket");
            continue;
        }

        if (setsockopt(fdSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0){
            perror("setsockopt");
            close(fdSocket);
            continue;
        }

        if (bind(fdSocket, ai->ai_addr, ai->ai_addrlen) < 0){
            perror("bind");
            close(fdSocket);
            continue;
        }
        break;
    }
    
    freeaddrinfo(gai);
    if (!ai){
        std::cerr << "Failed to bind Server" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (listen(fdSocket, 8) < 0){
        perror("listen");
        close(fdSocket);
        exit(EXIT_FAILURE);
    }
}

TCPSocketServer::~TCPSocketServer(){
    if (close(fdSocket)){
        perror("close");
        exit(EXIT_FAILURE);
    }
}

int TCPSocketServer::Accept(struct sockaddr* from, unsigned int* len) const{
    int clt;
    if ((clt = accept(fdSocket, from, len)) < 0){
        perror("accept");
        return -1;
    }
    return clt;
}

int TCPSocketServer::GetData(char* buffer, unsigned int len, int fd) const{
    int length;
    
    memset(buffer, 0, len);
    if ((length = recv(fd, buffer, len, 0)) < 0){
        perror("recv");
        return -1;
    }
    return length;
}

int TCPSocketServer::SendData(const char* data, unsigned int len, int fd) const{
    int length;

    if ((length = send(fd, data, len, 0)) < 0){
        perror("send");
        return -1;
    } 
    return length;
}

void TCPSocketServer::SendError(const std::string& errorCode, int fd) const{
    std::string data = "ERROR " + errorCode + "\n";
    SendData(data.c_str(), (unsigned int)data.size(), fd);
}

/*
|******************************|
|             Client           |
|******************************|
*/

TCPSocketClient::TCPSocketClient(const char* ip, const char* port){
    struct addrinfo hints, *gai, *ai;
    int err;

    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((err = getaddrinfo(ip, port, &hints, &gai))){
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }
    for (ai = gai; ai != NULL; ai = ai->ai_next){
        if ((fdSocket = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) < 0){
            perror("socket");
            continue;
        }
        if (connect(fdSocket, ai->ai_addr, ai->ai_addrlen) < 0){
            close(fdSocket);
            perror("connect");
            continue;
        }
        break;
    }   

    if (!ai){
        std::cout << "Can't connect to server " << ip << ":" << "port" << std::endl;
        exit(EXIT_FAILURE); 
    }
    freeaddrinfo(gai);
}

TCPSocketClient::~TCPSocketClient(){
    if (close(fdSocket)){
        perror("close");
        exit(EXIT_FAILURE);
    }
}

int TCPSocketClient::GetData(char* buffer, unsigned int len) const{
    int length;
    
    memset(buffer, 0, len);
    if ((length = recv(fdSocket, buffer, len, 0)) < 0){
        perror("recv");
        return -1;
    }
    return length;
}


int TCPSocketClient::SendData(const char* data, unsigned int len) const{
    int length;

    if ((length = send(fdSocket, data, len, 0)) < 0){
        perror("send");
        return -1;
    } 
    return length;
}


#endif