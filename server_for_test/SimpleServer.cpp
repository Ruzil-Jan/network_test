#include "SimpleServer.h"
#include "../needed_files/Utils.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>

SimpleServer::SimpleServer(int port)
    : port_(port), server_fd_(-1), running_(false) {}

SimpleServer::~SimpleServer()
{
    stop();
}

bool SimpleServer::start()
{
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0)
    {
        Utils::log("Server: socket() failed: " + std::string(strerror(errno)));
        return false;
    }

    // Allow reuse of address
    int opt = 1;
    if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        Utils::log("Server: setsockopt() failed: " + std::string(strerror(errno)));
        ::close(server_fd_);
        return false;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);

    if (bind(server_fd_, (sockaddr *)&address, sizeof(address)) < 0)
    {
        Utils::log("Server: bind() failed: " + std::string(strerror(errno)));
        ::close(server_fd_);
        return false;
    }

    if (listen(server_fd_, 3) < 0)
    {
        Utils::log("Server: listen() failed: " + std::string(strerror(errno)));
        ::close(server_fd_);
        return false;
    }

    running_ = true;
    server_thread_ = std::thread(&SimpleServer::serverLoop, this);

    Utils::log("Server started on port " + std::to_string(port_));
    return true;
}

void SimpleServer::stop()
{

    if (!running_)
        return;

    running_ = false;

    if (server_fd_ != -1) {
        ::close(server_fd_);
        server_fd_ = -1;
    }

    if (server_thread_.joinable())
    {
        Utils::log("Waiting for server thread to exit...");
        server_thread_.detach();
    }

    Utils::log("Server stopped cleanly.");
}

bool SimpleServer::isRunning() const
{
    return running_;
}

void SimpleServer::serverLoop()
{
    while (running_)
    {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_fd_, (sockaddr *)&client_addr, &client_len);
        if (client_fd < 0)
        {
            if (running_)
            {
                // Only log if we're still supposed to be running
                Utils::log("Server: accept() failed: " + std::string(strerror(errno)));
            }
            // Break out of loop if server is stopping
            break;
        }

        if (!running_)
        {
            // If we got a connection but server is stopping, close it
            ::close(client_fd);
            break;
        }

        Utils::log("Server: Client connected");
        handleClient(client_fd);
    }
    Utils::log("Server: Main loop exited");
}

void SimpleServer::handleClient(int client_fd)
{
    char buffer[1024];
    ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_read > 0)
    {
        buffer[bytes_read] = '\0';
        Utils::log("Server received: " + std::string(buffer));

        // Echo back with a prefix
        std::string response = "Echo: " + std::string(buffer);
        send(client_fd, response.c_str(), response.length(), 0);
        Utils::log("Server sent: " + response);
    }

    ::close(client_fd);
    Utils::log("Server: Client disconnected");
}