#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <vector>

#include <poll.h>

#include "../network/socket.hpp"
#include "game.hpp"

class Server{
    private:
        TCPSocketServer socket;
        Game game;

    public:
        Server(const char* port);

        void Run();

};  


#endif