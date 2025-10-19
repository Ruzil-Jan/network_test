#pragma once
#include <string>
#include <thread>
#include <atomic>

class SimpleServer {
private:
    int port_;
    int server_fd_;
    std::atomic<bool> running_;
    std::thread server_thread_;
    
    void serverLoop();
    void handleClient(int client_fd);

public:
    SimpleServer(int port);
    ~SimpleServer();
    
    bool start();
    void stop();
    bool isRunning() const;
};


class SimpleUDPServer {
private:
    int port_;
    int server_fd_;
    std::atomic<bool> running_;
    std::thread server_thread_;
    
    void serverLoop();

public:
    SimpleUDPServer(int port);
    ~SimpleUDPServer();
    
    bool start();
    void stop();
    bool isRunning() const;
};