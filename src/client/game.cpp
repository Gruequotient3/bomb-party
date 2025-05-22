#ifndef GAME_CPP
#define GAME_CPP

#include <iostream>
#include <cstring>

#include <unistd.h>

#include "game.hpp"

#define INIT_LIFES 3
#define BUF_SIZE 1024

Game::Game(){
    alive = true;
    lives = INIT_LIFES;
    username = "";
    tempUsername = "";
    state = NAMING;

    errorMap.insert({"00", "Error server"});
    errorMap.insert({"10", "Invalid argument"});
    errorMap.insert({"11", "Invalid command"});
    errorMap.insert({"01", "Authentification Neded"});
    errorMap.insert({"02", "Room is full"});
    errorMap.insert({"21", "Username not allowed"});
    errorMap.insert({"22", "Username already taken"});
    errorMap.insert({"23", "Mode not found"});
    errorMap.insert({"31", "Not enough players (min 2)"});
    errorMap.insert({"32", "Only game master can start the game"});
    errorMap.insert({"33", "Time's up - Kaboooom (-1 live)"});
    errorMap.insert({"35", "Not your turn"});
    errorMap.insert({"36", "Your aren't a player"});
}

int Game::InputHandler(const TCPSocketClient& socket){
    int status;
    unsigned int len;
    char buffer[BUF_SIZE];
    std::vector<std::string> parse;

    pollfd pfds[2];

    pfds[0].fd = socket.fdSocket;
    pfds[0].events = POLLIN;
    pfds[0].revents = 0;

    pfds[1].fd = STDIN_FILENO;
    pfds[1].events = POLLIN;
    pfds[1].revents = 0;

    if ((status = poll(pfds, 2, 0)) < 0){
        perror("poll");
        return 0;
    }

    if (status > 0){
        for (int i = 0; i < 2; ++i){
            if (!(pfds[i].revents & POLLIN)) continue;   
            if (pfds[i].fd == socket.fdSocket) {
                if ((len = socket.GetData(buffer, BUF_SIZE)) == 0){
                    std::cout << "Server is offline" << std::endl;
                    return -1;
                }
            }

            else {
                std::string msg;
                std::getline(std::cin, msg);
                bzero(buffer, BUF_SIZE);
                for (int i = 0; i < (int)msg.size()+1 && i < BUF_SIZE; ++i){
                    buffer[i] = msg[i];
                }
            }
            ParseMessage(buffer, BUF_SIZE, parse);
            switch(state){
                case NAMING:
                    if (pfds[i].fd == socket.fdSocket){
                        if (parse.size() < 1) continue;
                        if (parse[0] == "NAMEP") NamepCommandHandler(parse);
                        else if (parse[0] == "ERROR") ErrorHandler(parse);
                    } 
                    else{
                        if (parse.size() >= 1) tempUsername = parse[0];
                        std::string data = buffer;
                        data = "NAMEP " + data;
                        data += "\n";
                        socket.SendData(data.c_str(), data.size());
                    }
                    break;
                case WAITING_GAME:
                    if (pfds[i].fd == socket.fdSocket){
                        if (parse.size() < 1) continue;
                        if (parse[0] == "START") StartCommandHandler(parse);
                        else if (parse[0] == "PLYRS") PlayersCommandHandler(parse);
                        else if (parse[0] == "ALIVE") AliveCommandHandler(parse, socket);
                        else if (parse[0] == "ERROR") ErrorHandler(parse);
                    }
                    else{
                        std::string data = buffer;
                        data += "\n";
                        socket.SendData(data.c_str(), data.size());
                    }
                    break;
                case IN_GAME:
                    if (pfds[i].fd == socket.fdSocket){
                        if (parse.size() < 1) continue;
                        if (parse[0] == "GOVER") GameOverCommandHandler(parse);
                        else if (parse[0] == "DEADP") DeadCommandHandler(parse);
                        else if (parse[0] == "ROUND") RoundCommandHandler(parse);
                        else if (parse[0] == "SENDW") SendWordCommandHandler(parse);
                        else if (parse[0] == "ALIVE") AliveCommandHandler(parse, socket);
                        else if (parse[0] == "ERROR") ErrorHandler(parse);
                    }
                    else{
                        std::string data = buffer;
                        data = "SENDW " + data;
                        data += "\n";
                        socket.SendData(data.c_str(), data.size());
                    }
                    break;
            } 
        }
    }    
    return 0;
}




void Game::ParseMessage(const char* buffer, int len, std::vector<std::string>& message) const{
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

void Game::NamepCommandHandler(const std::vector<std::string>& cmd){
    if (cmd.size() != 3) return;
    else if (cmd[1] == tempUsername) {
        state = WAITING_GAME;
        username = cmd[1];
        std::cout << "You joined the room" << std::endl;
    }
    else if (cmd[2] != "J" || cmd[2] != "S") return;
    else std::cout << cmd[2] << " has joined the room as " << (cmd[2] == "J" ? "Player" : "Spectator");
}

void Game::AliveCommandHandler(const std::vector<std::string>& cmd, const TCPSocketClient& socket){
    if (cmd.size() != 1) return;
    else{
        std::string data = "ALIVE\n";
        socket.SendData(data.c_str(), data.size());
    }
}

void Game::StartCommandHandler(const std::vector<std::string>& cmd){
    if (cmd.size() != 1) return;
    alive = true;
    lives = INIT_LIFES;
    state = IN_GAME;
    std::cout << "--- Game Started (You have " << lives << " lives) ---" << std::endl;
}

void Game::PlayersCommandHandler(const std::vector<std::string>& cmd){
    bool first = true;
    std::cout << "Player in room : [";
    for (int i = 1; i < (int)cmd.size(); ++i){
        if (!first) 
        std::cout << "-";
        first = false;
        std::cout << cmd[i];
    }
    std::cout << "]" << std::endl;
}

void Game::RoundCommandHandler(const std::vector<std::string>& cmd){
    if (cmd.size() != 3) return;
    if (cmd[2] == username) std::cout << "Your turn : " << cmd[1] << std::endl;
    else std::cout << cmd[2] << "'s turn : " << cmd[1] << std::endl; 
}

void Game::SendWordCommandHandler(const std::vector<std::string>& cmd){
    if (cmd.size() != 3) return;
    if (cmd[2] == "C") std::cout << "- " << cmd[1] << " - Correct" << std::endl;
    else std::cout << "- " << cmd[1] << " - Incorrect" << std::endl;
}

void Game::GameOverCommandHandler(const std::vector<std::string>& cmd){
    if (cmd.size() != 2) return;
    if (cmd[1] == username) std::cout << "--- Good Job - You won ---" << std::endl;
    else std::cout << "--- " << cmd[1] << " won ---" << std::endl;
    
    state = WAITING_GAME;
    alive = true;
    lives = INIT_LIFES;
}

void Game::DeadCommandHandler(const std::vector<std::string>& cmd){
    if (cmd.size() != 2) return;
    if (cmd[1] != username){
        std::cout << "Player " << cmd[1] << " is dead" << std::endl;
    }
}

void Game::ErrorHandler(const std::vector<std::string>& cmd){
    if (cmd.size() != 2) return;
    auto it = errorMap.find(cmd[1]);
    if (it == errorMap.end()) return;
    if (it->first == "33") --lives;
    std::cout << "Error " << it->first << ": " << it->second << std::endl;
}

int Game::Update(const TCPSocketClient& socket){
    static bool first = true;
    
    if (InputHandler(socket) < 0) return -1;
    switch(state){
        case NAMING:
            if (first){
                std::cout << "Enter username and mode which you want to be (J: Player, S: Spectator)" << std::endl;
                first = false;
            }
            break;
        case WAITING_GAME:
            break;
        case IN_GAME:
            if (alive && lives == 0){
                alive = false;
                std::cout << "--- Oh Nooooo - You're Dead ---" << std::endl;
                std::string data = "DEADP\n";
                socket.SendData(data.c_str(), data.size()+1);
            }
            break;
    }

    return 0;
}

#endif