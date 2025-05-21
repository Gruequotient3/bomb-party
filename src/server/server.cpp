#ifndef SERVER_CPP
#define SERVER_CPP

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "server.hpp"


Server::Server(const char* port) : socket{port} { 
    std::cout << "*** Bomb Party - Server Started ***\n" << std::endl;
}

void Server::Run(){
    for (;;){
        pollfd fdServer;
        fdServer.fd = socket.fdSocket;
        fdServer.events = POLLIN;
        
        // New Connection Handler
        int state = poll(&fdServer, 1, 0);
        if (state < 0){
            perror("poll");
            break;
        }

        if (state > 0){
            void* addr;
            sockaddr_storage from;
            unsigned int len = sizeof(from);
            char ip[INET6_ADDRSTRLEN];
            in_port_t port;
            
            int clt = socket.Accept((struct sockaddr*)&from, &len);
            if (clt != -1){
                addr = (from.ss_family == AF_INET)
                       ? (void *)&(((struct sockaddr_in *)&from)->sin_addr)
                       : (void *)&(((struct sockaddr_in6 *)&from)->sin6_addr);
                    inet_ntop(from.ss_family, addr, ip, sizeof(ip));
                    port = (from.ss_family == AF_INET)
                       ? ((struct sockaddr_in *)&from)->sin_port
                       : ((struct sockaddr_in6 *)&from)->sin6_port;
                    printf("> Connection from %s:%d\n", ip, ntohs(port));
                
                game.AddConnection(clt);
            }
        }
        game.Update(socket);
    }
}

#endif