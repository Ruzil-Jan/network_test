#pragma once

#include "ISocket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

#pragma once
#include "../headers/network/ISocket.h"
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>

class UDPSocket : public ISocket
{
private:
    std::string address_;
    int port_;
    int sockfd_;
    sockaddr_in server_addr_;

public:
    UDPSocket(const std::string &address, int port);
    virtual ~UDPSocket() override;

    // ISocket interface implementation
    virtual bool open() override;
    virtual void close() override;
    virtual bool send(const std::string &data) override;
    virtual std::string receive() override;

    // Additional UDP-specific methods
    bool isConnected() const;
    std::string receive(size_t max_size);
    std::string receiveWithTimeout(int timeout_seconds, size_t max_size = 4096);
    std::string receiveUntil(const std::string &delimiter, size_t max_size = 65536);

    // Socket configuration methods
    int getSocketFd() const;
    bool setSocketOption(int level, int optname, const void *optval, socklen_t optlen);
    bool setReceiveTimeout(int seconds);
    bool setSendTimeout(int seconds);

    // Connection info
    std::string getAddress() const;
    int getPort() const;
};
