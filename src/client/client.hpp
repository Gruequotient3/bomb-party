#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "../network/socket.hpp"
#include "game.hpp"

class Client{
    private:
        TCPSocketClient socket;
        Game game;

    public:
        Client(const char* ip, const char* port);

        void Run();
};

#endif