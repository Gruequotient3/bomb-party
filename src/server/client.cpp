#ifndef CLIENT_CPP
#define CLIENT_CPP

#include "client.hpp"

Client::Client(int fd, const std::string& username) : fd{fd} { 
    connected = true;
    answerAlive = true;
    hasSpeak = true;

    alive = true;
    aliveTimer = 3;
    
    this->username = username;
}

int Client::GetFd() const{ 
    return fd; 
}
bool Client::IsConnected() const{
    return connected;
}
bool Client::HasAnswerAlive() const{
    return answerAlive;
}
bool Client::HasSpeak() const{
    return hasSpeak;
}
bool Client::IsAlive() const{ 
    return alive; 
}
int Client::GetAliveTimer() const{ 
    return aliveTimer; 
}
const std::string& Client::GetUsername() const{
    return username;
}

void Client::SetConnected(bool value){
    connected = value;
}
void Client::SetAnswerAlive(bool value){
    answerAlive = value;
}
void Client::SetHasSpeak(bool value){
    hasSpeak = value;
}
void Client::SetAlive(bool value) {
    alive = value;
}
void Client::SetAliveTimer(int value) {
    aliveTimer = value;
}
void Client::SetUsername(const std::string& username){
    this->username = username;
}

#endif