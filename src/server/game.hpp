#ifndef GAME_HPP
#define GAME_HPP

#include <map>
#include <vector>
#include <string>
#include <ctime>

#include "../network/socket.hpp"
#include "client.hpp"

enum GameState{
    WAITING_GAME,
    IN_GAME,
};

class Game{
    private:
        std::vector<int> waiters;
        std::map<int, Client> players;
        std::map<int, Client> viewers;

        std::vector<std::string> words;
        std::string currentletters;

        Client* leader;
        std::map<int, Client>::iterator current;

        GameState state;

        int timer;
        int reinitTimer; 
        int minTimer;
        int maxTimer;

        time_t oldTimestamp;
        time_t timestamp;
       
        time_t oldTimestampAlive;
        time_t timestampAlive;

        bool gameOver;
        bool firstRound;

        
        void ParseMessage(char* buffer, int len,  std::vector<std::string>& message) const;        
        int CheckName(const std::string& name) const;
        Client* CheckGameOver();
        bool CheckWord(std::string word);
        
        bool NamepCommandHandler(const std::vector<std::string>& cmd, std::vector<int>::iterator it, const TCPSocketServer& server);
        void AliveCommandHandler(const std::vector<std::string>& cmd, Client& client, const TCPSocketServer& server);
        bool StartCommandHandler(const std::vector<std::string>& cmd, Client& client, const TCPSocketServer& server);
        void DeadCommandHandler(const std::vector<std::string>& cmd, Client& client, const TCPSocketServer& server);
        void SendWordCommandHandler(const std::vector<std::string>& cmd, Client& client, const TCPSocketServer& server);
        
        void ClientsInputHandler(const TCPSocketServer& server);
        void RoundStart(const TCPSocketServer& server);
        void RoundEnd(const TCPSocketServer& server);

        void SendPlayersCommand(const TCPSocketServer& server);
        void BroadcastMessage(const std::string& message, const TCPSocketServer& server);

        void FillWorldList();
        std::string GenerateLetterSequence();

    public:
        Game();

        void AddConnection(int fd);
        void Update(const TCPSocketServer& server);
        

};

#endif