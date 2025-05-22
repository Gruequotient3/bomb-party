#ifndef CLIENT_CPP
#define CLIENT_CPP

#include <iostream>

#include <unistd.h>

#include "client.hpp"

Client::Client(const char* ip, const char* port) : socket{ip, port}{
    std::cout << "*** Bomb Party - Client Connected ***\n" << std::endl;
}

void Client::Run(){
    for (;;){
        if (game.Update(socket) < 0) break;
    }
}

#endif