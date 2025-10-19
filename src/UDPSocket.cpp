#include "../headers/network/UDPSocket.h"
#include "../needed_files/Utils.h"


UDPSocket::UDPSocket(const std::string& address, int port)
    : address_(address), port_(port), sockfd_(-1) {}

UDPSocket::~UDPSocket() {
    close();
}

bool UDPSocket::open() {
    sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_ < 0) {
        Utils::log("Error: socket() failed: " + std::string(strerror(errno)));
        return false;
    }

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_);
    
    if (inet_pton(AF_INET, address_.c_str(), &serv_addr.sin_addr) <= 0) {
        Utils::log("Error: invalid IP address: " + address_);
        ::close(sockfd_);
        sockfd_ = -1;
        return false;
    }

    // Store the server address for sending/receiving
    server_addr_ = serv_addr;
    Utils::log("UDP socket created for " + address_ + ":" + std::to_string(port_));
    return true;
}

void UDPSocket::close() {
    if (sockfd_ != -1) {
        ::close(sockfd_);
        sockfd_ = -1;
        Utils::log("UDP socket closed.");
    }
}

bool UDPSocket::send(const std::string& data) {
    if (sockfd_ < 0) {
        Utils::log("Error: socket is not open.");
        return false;
    }

    if (data.empty()) {
        return true; // Nothing to send
    }

    size_t total_sent = 0;
    const char* buffer = data.c_str();
    size_t data_size = data.size();

    while (total_sent < data_size) {
        ssize_t sent = ::sendto(sockfd_, buffer + total_sent, data_size - total_sent, MSG_NOSIGNAL,
                                (sockaddr*)&server_addr_, sizeof(server_addr_));
        
        if (sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Socket buffer full, try again
                continue;
            }
            Utils::log("Error: sendto() failed: " + std::string(strerror(errno)));
            return false;
        }
        
        total_sent += sent;
    }

    return true;
}

std::string UDPSocket::receive() {
    return receive(4096); // Default buffer size
}

bool UDPSocket::isConnected() const {
    // UDP is connectionless; return true if socket is open
    return sockfd_ != -1;
}

std::string UDPSocket::receive(size_t max_size) {
    if (sockfd_ < 0) {
        Utils::log("Error: socket is not open.");
        return "";
    }

    char* buffer = new char[max_size];
    sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(sender_addr);

    ssize_t n = ::recvfrom(sockfd_, buffer, max_size, 0, (sockaddr*)&sender_addr, &addr_len);
    
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No data available right now
            delete[] buffer;
            return "";
        }
        Utils::log("Error: recvfrom() failed: " + std::string(strerror(errno)));
        delete[] buffer;
        return "";
    }
    
    std::string result(buffer, n);
    delete[] buffer;
    return result;
}

std::string UDPSocket::receiveWithTimeout(int timeout_seconds, size_t max_size) {
    if (sockfd_ < 0) {
        Utils::log("Error: socket is not open.");
        return "";
    }

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(sockfd_, &read_fds);

    struct timeval timeout;
    timeout.tv_sec = timeout_seconds;
    timeout.tv_usec = 0;

    int activity = select(sockfd_ + 1, &read_fds, nullptr, nullptr, &timeout);
    
    if (activity < 0) {
        Utils::log("Error: select() failed: " + std::string(strerror(errno)));
        return "";
    }
    
    if (activity == 0) {
        Utils::log("Receive timeout.");
        return "";
    }

    if (FD_ISSET(sockfd_, &read_fds)) {
        return receive(max_size);
    }

    return "";
}

std::string UDPSocket::receiveUntil(const std::string& delimiter, size_t max_size) {
    if (sockfd_ < 0) {
        Utils::log("Error: socket is not open.");
        return "";
    }

    std::string result;
    char buffer[1024];
    sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(sender_addr);

    while (result.size() < max_size) {
        ssize_t n = ::recvfrom(sockfd_, buffer, sizeof(buffer), 0, (sockaddr*)&sender_addr, &addr_len);
        
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            Utils::log("Error: recvfrom() failed: " + std::string(strerror(errno)));
            break;
        }
        
        result.append(buffer, n);
        
        // Check if delimiter is found
        size_t pos = result.find(delimiter);
        if (pos != std::string::npos) {
            // Return data up to and including delimiter
            return result.substr(0, pos + delimiter.length());
        }
    }
    
    return result;
}

int UDPSocket::getSocketFd() const {
    return sockfd_;
}

bool UDPSocket::setSocketOption(int level, int optname, const void* optval, socklen_t optlen) {
    if (sockfd_ < 0) {
        Utils::log("Error: socket is not open.");
        return false;
    }
    
    if (setsockopt(sockfd_, level, optname, optval, optlen) < 0) {
        Utils::log("Error: setsockopt() failed: " + std::string(strerror(errno)));
        return false;
    }
    
    return true;
}

bool UDPSocket::setReceiveTimeout(int seconds) {
    struct timeval timeout;
    timeout.tv_sec = seconds;
    timeout.tv_usec = 0;
    return setSocketOption(SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}

bool UDPSocket::setSendTimeout(int seconds) {
    struct timeval timeout;
    timeout.tv_sec = seconds;
    timeout.tv_usec = 0;
    return setSocketOption(SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
}

std::string UDPSocket::getAddress() const {
    return address_;
}

int UDPSocket::getPort() const {
    return port_;
}