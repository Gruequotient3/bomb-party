#ifndef GAME_HPP
#define GAME_HPP

#include <string>
#include <vector>

#include <poll.h>

#include "../network/socket.hpp"

enum ClientState{
    NAMING,
    WAITING_GAME,
    IN_GAME,
};

class Game{
    private:
        std::map<std::string, std::string> errorMap;

        bool alive;

        int lives;
        std::string username;
        std::string tempUsername;
        ClientState state;

        int InputHandler(const TCPSocketClient& socket);
        void ParseMessage(const char* buffer, int len, std::vector<std::string>& message) const;

        void NamepCommandHandler(const std::vector<std::string>& cmd);
        void AliveCommandHandler(const std::vector<std::string>& cmd, const TCPSocketClient& socket);
        void StartCommandHandler(const std::vector<std::string>& cmd);
        void PlayersCommandHandler(const std::vector<std::string>& cmd);
        void RoundCommandHandler(const std::vector<std::string>& cmd);
        void SendWordCommandHandler(const std::vector<std::string>& cmd);
        void GameOverCommandHandler(const std::vector<std::string>& cmd);
        void DeadCommandHandler(const std::vector<std::string>& cmd);
        void ErrorHandler(const std::vector<std::string>& cmd);

    public:
        Game();

        int Update(const TCPSocketClient& socket);

        


};


#endif