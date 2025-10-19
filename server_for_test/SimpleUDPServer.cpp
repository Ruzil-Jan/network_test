#include "SimpleServer.h"
#include "../needed_files/Utils.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>


SimpleUDPServer::SimpleUDPServer(int port)
    : port_(port), server_fd_(-1), running_(false) {}

SimpleUDPServer::~SimpleUDPServer()
{
    stop();
}

bool SimpleUDPServer::start()
{
    server_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
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

    running_ = true;
    server_thread_ = std::thread(&SimpleUDPServer::serverLoop, this);

    Utils::log("UDP Server started on port " + std::to_string(port_));
    return true;
}

void SimpleUDPServer::stop()
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

    Utils::log("UDP Server stopped cleanly.");
}

bool SimpleUDPServer::isRunning() const
{
    return running_;
}

void SimpleUDPServer::serverLoop()
{
    char buffer[1024];
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    while (running_)
    {
        // Receive datagram
        ssize_t bytes_read = recvfrom(server_fd_, buffer, sizeof(buffer) - 1, 0,
                                      (sockaddr *)&client_addr, &client_len);

        if (bytes_read < 0)
        {
            if (running_)
            {
                // Only log if we're still supposed to be running
                Utils::log("Server: recvfrom() failed: " + std::string(strerror(errno)));
            }
            continue;
        }

        if (!running_)
        {
            break;
        }

        if (bytes_read > 0)
        {
            buffer[bytes_read] = '\0';
            Utils::log("Server received: " + std::string(buffer));

            // Echo back with a prefix
            std::string response = "Echo: " + std::string(buffer);
            sendto(server_fd_, response.c_str(), response.length(), 0,
                   (sockaddr *)&client_addr, client_len);
            Utils::log("Server sent: " + response);
        }
    }
    Utils::log("Server: Main loop exited");
}