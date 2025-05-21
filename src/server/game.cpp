#ifndef GAME_CPP
#define GAME_CPP

#include <iostream>
#include <fstream>

#include <poll.h>
#include <unistd.h>

#include "game.hpp"

#define BUF_SIZE 1024

#define MAX_PLAYER 16
#define MIN_PLAYER 2

#define INIT_ALIVE_TIMER 3

Game::Game() {
    leader = nullptr; 
    current = players.end();
    state = WAITING_GAME;

    timer = 0;
    reinitTimer = 5;
    minTimer = 10;
    maxTimer = 30;

    timestamp = (time_t)0;
    oldTimestamp = (time_t)0;

    timestampAlive = (time_t)time(NULL);
    oldTimestampAlive = (time_t)timestampAlive;

    gameOver = false;
    firstRound = true;

    FillWorldList();
}

void Game::AddConnection(int fd){
    waiters.push_back(fd);
}

void Game::ClientsInputHandler(const TCPSocketServer& server){
    int status, len;
    char buffer[BUF_SIZE];
    std::vector<std::string> parse;

    // Check Waiters
    auto itWaiters = waiters.begin();
    for (; itWaiters != waiters.end(); ++itWaiters){
        pollfd pfd;
        pfd.fd = *itWaiters;
        pfd.events = POLLIN;
        
        if ((status = poll(&pfd, 1, 0)) < 0){
            perror("poll");
            continue;
        }

        if (status > 0){
            if ((len = server.GetData(buffer, BUF_SIZE, pfd.fd)) == 0){
                close(*itWaiters);
                auto it = itWaiters--;
                waiters.erase(it); 
                std::cout << "> A Client has been deconnected" << std::endl;
                continue;
            }
            ParseMessage(buffer, BUF_SIZE, parse);
            if (parse.size() < 1) {
                server.SendError("11", pfd.fd);
                continue; 
            }

            // Handle command for not named connexion 
            if (parse[0] == "NAMEP") {
                if (NamepCommandHandler(parse, itWaiters, server)){
                    auto it = itWaiters--;
                    waiters.erase(it);
                };
            }
            else server.SendError("11", pfd.fd);
        }
    }

    // Check Players
    auto itPlayers = players.begin();
    for (; itPlayers != players.end(); ++itPlayers){
        pollfd pfd; 
        pfd.fd = itPlayers->first;
        pfd.events = POLLIN;

        
        if (!leader) leader = &itPlayers->second;
        if ((status = poll(&pfd, 1, 0)) < 0){
            perror("poll");
            continue;
        }

        if (status > 0){
            switch(state){
                case WAITING_GAME:
                    if ((len = server.GetData(buffer, BUF_SIZE, pfd.fd)) == 0){
                        close(itPlayers->first);
                        if (itPlayers->first == leader->GetFd()) leader = nullptr;
                        auto it = itPlayers--;
                        std::cout << "> Deconnexion of player " << it->second.GetUsername() << std::endl; 
                        players.erase(it->first);
                        SendPlayersCommand(server);
                        continue;
                    }
                    ParseMessage(buffer, BUF_SIZE, parse);
                    if (parse.size() < 1) {
                        server.SendError("11", pfd.fd);
                        continue;
                    }

                    if (parse[0] == "START"){
                        if (StartCommandHandler(parse, itPlayers->second, server)) return;
                    }
                    else if (parse[0] == "ALIVE") AliveCommandHandler(parse, itPlayers->second, server);
                    else server.SendError("11", pfd.fd);

                    break;

                case IN_GAME:
                    itPlayers->second.SetHasSpeak(true);
                    if (!itPlayers->second.IsConnected()) continue;
                    if ((len = server.GetData(buffer, BUF_SIZE, pfd.fd)) == 0){
                        itPlayers->second.SetConnected(false);
                        itPlayers->second.SetAlive(false);
                        BroadcastMessage("DEADP " + itPlayers->second.GetUsername() + "\n", server);
                        continue;
                    }

                    ParseMessage(buffer, BUF_SIZE, parse);
                    if (parse.size() < 1) {
                        server.SendError("11", pfd.fd);
                        continue;
                    }

                    if (parse[0] == "DEADP") DeadCommandHandler(parse, itPlayers->second, server);
                    else if (parse[0] == "SENDW") SendWordCommandHandler(parse, itPlayers->second, server);
                    else if (parse[0] == "ALIVE") AliveCommandHandler(parse, itPlayers->second, server);
                    else server.SendError("11", pfd.fd);
                    break;
            }
        }
    }

    // Check viewers
    auto itViewers = viewers.begin();
    for (; itViewers != viewers.end(); ++itViewers){
        pollfd pfd; 
        pfd.fd = itViewers->first;
        pfd.events = POLLIN;

        
        if (!leader) leader = &itViewers->second;
        if ((status = poll(&pfd, 1, 0)) < 0){
            perror("poll");
            continue;
        }

        if (status > 0){
            if ((len = server.GetData(buffer, BUF_SIZE, pfd.fd)) == 0){
                close(itViewers->first);
                auto it = itViewers--;
                std::cout << "> Deconnexion of viewer " << it->second.GetUsername() << std::endl;
                viewers.erase(it->first);
                SendPlayersCommand(server);
                continue;
            }
            ParseMessage(buffer, BUF_SIZE, parse);
            if (parse.size() < 1){
                server.SendError("11", pfd.fd);
                continue;
            }

            if (parse[0] == "ALIVE") AliveCommandHandler(parse, itViewers->second, server);
            else server.SendError("36", pfd.fd);
            break;
        }
    }
}




void Game::ParseMessage(char* buffer, int len, std::vector<std::string>& message) const{
    std::string parse = "";
    std::string escapeChar = " -[]";
    char prev = ' ';
    bool inBracket = false;
    bool skip;
    
    message.clear();
    for (int i = 0;  i < len; ++i){
        if (buffer[i] == '\0' || buffer[i] == '\n'){
            if (parse != "") message.push_back(parse);
            break;
        }

        if (prev == ' ' && buffer[i] == '['){
            inBracket = true;
        }

        else if (inBracket){
            if (buffer[i] == ']'){
                inBracket = false;
                message.push_back(parse);
                parse = "";
            }
            
            if (buffer[i] == '-'){
                message.push_back(parse);
                parse = "";
            }
        }

        if (prev != ' ' && buffer[i] == ' '){
            message.push_back(parse);
            parse = "";
        }

        skip = false;
        for (int j = 0; j < (int)escapeChar.size(); ++j){
            if (buffer[i] == escapeChar[j]){
                skip = true;
                break;
            }
        }

        if (buffer[i] == '\n' || buffer[i] == 13){
            skip = true;
        }

        if (!skip){
            parse += buffer[i];
        }

        prev = buffer[i];
    }
}

int Game::CheckName(const std::string& name) const{
    if (name.size() < 1 || name.size() > 10) return -1;

    for (int i = 0; i < (int)name.size(); ++i){
        if (!(name[i] >= 48 && name[i] < 58) && 
            !(name[i] >= 65 && name[i] < 91) &&
            !(name[i] >= 97 && name[i] < 123)) return -1;
    }

    auto itClient = players.begin();
    for (; itClient != players.end(); ++itClient){
        if (itClient->second.GetUsername() == name) return -2;
    }

    itClient = viewers.begin();
    for (; itClient != viewers.end(); ++itClient){
        if (itClient->second.GetUsername() == name) return -2;
    }
    return 0;
}

Client* Game::CheckGameOver(){
    Client* client = nullptr;
    auto itPlayers = players.begin();
    for (; itPlayers != players.end(); ++itPlayers){
        if (itPlayers->second.IsAlive()){
            if (!client) client = &itPlayers->second;
            else return nullptr;
        }
    }
    return client;
}

bool Game::CheckWord(std::string word){
    bool containSubstr = false;
    int letterIndex = 0;
    for (int i = 0; i < (int)word.size(); ++i){
        if (currentletters[letterIndex] != word[i]){
            letterIndex = 0;
            containSubstr = false;
            continue;
        }
        ++letterIndex;
        
        if (letterIndex == (int)currentletters.size()){
            containSubstr = true;
            break;
        }
    }

    if (containSubstr)
        for (auto w : words)
            if (word == w) return true;
    return false;
}


bool Game::NamepCommandHandler(const std::vector<std::string>& cmd, std::vector<int>::iterator it, const TCPSocketServer& server){
    if (cmd.size() != 3) server.SendError("10", *it);
    else if (cmd[2] != "J" && cmd[2] != "S") server.SendError("23", *it);
    else{
        int status = CheckName(cmd[1]);
        switch (status){
            case -1:
                server.SendError("21", *it);
                break;
            case -2:
                server.SendError("22", *it);
                break;
            default:
                if (cmd[2] == "J") {
                    if (players.size() < MAX_PLAYER){
                        players.insert({*it, Client{*it, cmd[1]}});
                        BroadcastMessage(cmd[0] + " " + cmd[1] + " " + cmd[2] + "\n", server);
                        SendPlayersCommand(server);
                        return true;
                    }
                    else server.SendError("02", *it);
                }
                else {
                    viewers.insert( {*it, Client{*it, cmd[1]}} );
                    BroadcastMessage(cmd[0] + " " + cmd[1] + " " + cmd[2] + "\n", server);
                    return true;
                }
        }
    }
    return false;
}

void Game::AliveCommandHandler(const std::vector<std::string>& cmd, Client& client, const TCPSocketServer& server){
    if (cmd.size() != 1) server.SendError("10", client.GetFd());
    else client.SetAnswerAlive(true);
}

bool Game::StartCommandHandler(const std::vector<std::string>& cmd, Client& client, const TCPSocketServer& server){
    if (cmd.size() != 1) server.SendError("10", client.GetFd());
    else if (!leader) server.SendError("00", client.GetFd());
    else{
        if (players.size() < MIN_PLAYER){
            server.SendError("31", client.GetFd());
            return false;
        }
        if (client.GetFd() != leader->GetFd()){
            server.SendError("32", client.GetFd());
            return false;
        }
        BroadcastMessage("START\n", server);
        firstRound = true;
        state = IN_GAME;
        RoundStart(server);
        return true;
    }
    return  false;
}

void Game::DeadCommandHandler(const std::vector<std::string>& cmd, Client& client, const TCPSocketServer& server){
    if (cmd.size() != 1) server.SendError("10", client.GetFd());
    else if (!client.IsAlive()) return;
    else{
        client.SetAlive(false);
        BroadcastMessage("DEADP " + client.GetUsername() + "\n", server);
        
        Client* winner;
        if ((winner = CheckGameOver())){
            BroadcastMessage("GOVER " + winner->GetUsername() + "\n", server);
            gameOver = true;
            state = WAITING_GAME;
            return;
        }    
    }
}

void Game::SendWordCommandHandler(const std::vector<std::string>& cmd, Client& client, const TCPSocketServer& server){
    if (cmd.size() != 2) server.SendError("10", client.GetFd());
    else if (current->first != client.GetFd()) server.SendError("35", client.GetFd());
    else{
        std::string word = cmd[1];
        std::cout << word << std::endl;
        if (CheckWord(word)) {
            BroadcastMessage("SENDW " + word + " C\n", server);
            RoundEnd(server);
            if (!gameOver) RoundStart(server);
        }
        else BroadcastMessage("SENDW " + word + " I\n", server);
    }
}

void Game::RoundStart(const TCPSocketServer& server){
    // Setup First Round
    if (firstRound) {
        current = players.begin();

        gameOver = false;
        firstRound = false;

        for (auto p : players){
            p.second.SetAlive(true);
        }
    }
    
    timestamp = time(NULL);
    oldTimestamp = timestamp;
    
    // Setup Timer
    if (timer <= 0){
        timer = 2;//rand() % (maxTimer + 1 - minTimer) + minTimer;
    }
    else if (timer <= 5) timer = minTimer;

    
    current->second.SetHasSpeak(false);
    currentletters = GenerateLetterSequence();
    BroadcastMessage("ROUND " + currentletters + " " + current->second.GetUsername() + "\n", server);
}

void Game::RoundEnd(const TCPSocketServer& server){
    Client* client = nullptr;
    if ((client = CheckGameOver())){
        BroadcastMessage("GOVER " + client->GetUsername() + "\n", server);
        gameOver = true;
        state = WAITING_GAME;
        return;
    }

    if (!current->second.HasSpeak()){
        std::string data = "ALIVE\n";
        server.SendData(data.c_str(), (unsigned int)data.size(), current->first);
        current->second.SetAnswerAlive(false);
        current->second.SetAliveTimer(INIT_ALIVE_TIMER);
    }

    while (true){
        ++current;
        if (current == players.end()){
            current = players.begin();
        }
        if(current->second.IsAlive()) break;
    }
}

void Game::SendPlayersCommand(const TCPSocketServer& server){
    std::string player = "PLYRS [";
    auto itPlayers = players.begin(); 
    for (; itPlayers != players.end(); ++itPlayers){
        if (itPlayers != players.begin()) player += "-";
        player += itPlayers->second.GetUsername();
    }
    player += "]\n";
    BroadcastMessage(player, server);
}

void Game::BroadcastMessage(const std::string& message, const TCPSocketServer& server){
    auto itPlayers = players.begin();
    for (; itPlayers != players.end(); ++itPlayers){
        if (itPlayers->second.IsConnected())
            server.SendData(message.c_str(), (unsigned int)message.size(), itPlayers->first);
    }

    auto itViewers = viewers.begin();
    for (; itViewers != viewers.end(); ++itViewers){
        server.SendData(message.c_str(), (unsigned int)message.size(), itViewers->first);
    }
}

void Game::Update(const TCPSocketServer& server){
    ClientsInputHandler(server);
    timestampAlive = time(NULL);
    switch (state){
        case WAITING_GAME:
            {
            if (gameOver == true){
                gameOver = false;
                current = players.end();
            }

            // Alive Handler
            auto itPlayers = players.begin();
            for (; itPlayers != players.end(); ++itPlayers){
                Client& client = itPlayers->second;

                if (client.GetAliveTimer() > 0){
                    client.SetAliveTimer(client.GetAliveTimer() - (timestampAlive - oldTimestampAlive));
                    continue;
                } 

                if (!client.HasAnswerAlive()){
                    close(itPlayers->first);
                    auto it = itPlayers--;
                    if (it->first == leader->GetFd()) leader = nullptr;
                    std::cout << "> Deconnexion of player " << it->second.GetUsername() << std::endl;
                    players.erase(it);
                    SendPlayersCommand(server);
                    continue; 
                }

                std::string msg = "ALIVE\n";
                server.SendData(msg.c_str(), (unsigned int)msg.size(), client.GetFd());
                client.SetAliveTimer(INIT_ALIVE_TIMER);
                client.SetAnswerAlive(false);
            }

            auto itViewers = viewers.begin();
            for (; itViewers != viewers.end(); ++itViewers){
                Client& client = itViewers->second;

                if (client.GetAliveTimer() > 0){
                    client.SetAliveTimer(client.GetAliveTimer() - (timestampAlive - oldTimestampAlive));
                    continue;
                } 

                if (!client.HasAnswerAlive()){
                    close(itViewers->first);
                    auto it = itViewers--;
                    std::cout << "> Deconnexion of viewer " << it->second.GetUsername() << std::endl;
                    viewers.erase(it);
                    continue; 
                }

                std::string msg = "ALIVE\n";
                server.SendData(msg.c_str(), (unsigned int)msg.size(), client.GetFd());
                client.SetAliveTimer(INIT_ALIVE_TIMER);
                client.SetAnswerAlive(false);
            }
            break;
            }   
        case IN_GAME:
            {
            // Alive Handler
            auto itPlayers = players.begin();
            for (; itPlayers != players.end(); ++itPlayers){
                Client& client = itPlayers->second;

                if (!client.IsConnected()) continue;
                if (client.HasSpeak() || current->first == client.GetFd()) continue;
                if (client.GetAliveTimer() > 0){
                    client.SetAliveTimer(client.GetAliveTimer() -( timestampAlive - oldTimestampAlive));
                    continue;
                } 

                if (!client.HasAnswerAlive()){
                    client.SetAlive(false);
                    client.SetConnected(false);
                    std::cout << "salut" << std::endl;
                    BroadcastMessage("DEADP " + client.GetUsername() + "\n", server);
                    continue; 
                }
            }

            if (timer <= 0){
                server.SendError("33\n", current->first);
                RoundEnd(server);
                if (!gameOver) RoundStart(server);
                break;
            }

            timestamp = time(NULL);
            timer -= timestamp - oldTimestamp;
            oldTimestamp = timestamp;
            break;
            }
    }
    oldTimestampAlive = timestampAlive;

}

void Game::FillWorldList(){
    std::ifstream file("src/server/res/french");
    std::string text;
    while (std::getline(file, text)){
        text[text.find('\n')] = '\0';
        words.push_back(text);
    }
}

std::string Game::GenerateLetterSequence(){
    srand(time(NULL));

    while (true){
        int pos = rand() % (int)words.size();
        std::string word = words[pos];
        int len  = 2 + rand() % 3;
        int start = rand() % ((int)words.size() - len);
        if (start + len <= (int)word.size()){
            return word.substr(start, len);
        }
    }
}

#endif