#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client{
    protected:
        int fd;

        bool connected;
        bool answerAlive;
        bool hasSpeak;

        int aliveTimer;
        
        bool alive;
        std::string username;
        
    public:
        Client(int fd, const std::string& username);

        void Reset();

        int GetFd() const;
        bool IsConnected() const;
        bool HasAnswerAlive() const;
        bool HasSpeak() const;
        bool IsAlive() const;
        int GetAliveTimer() const;
        const std::string& GetUsername() const;

        void SetConnected(bool value);
        void SetAnswerAlive(bool value);
        void SetHasSpeak(bool value);

        void SetAlive(bool value);
        void SetAliveTimer(int value);
        void SetUsername(const std::string& username);
};

#endif