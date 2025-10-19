#include "../headers/network/TCPSocket.h"
#include "../needed_files/Utils.h"



TCPSocket::TCPSocket(const std::string& address, int port)
    : address_(address), port_(port), sockfd_(-1) {}

TCPSocket::~TCPSocket() {
    close();
}

bool TCPSocket::open() {
    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
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

    if (connect(sockfd_, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        Utils::log("Error: connect() failed: " + std::string(strerror(errno)));
        ::close(sockfd_);
        sockfd_ = -1;
        return false;
    }

    Utils::log("TCP connection established to " + address_ + ":" + std::to_string(port_));
    return true;
}

void TCPSocket::close() {
    if (sockfd_ != -1) {
        ::close(sockfd_);
        sockfd_ = -1;
        Utils::log("TCP socket closed.");
    }
}

bool TCPSocket::send(const std::string& data) {
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
        ssize_t sent = ::send(sockfd_, buffer + total_sent, 
                            data_size - total_sent, MSG_NOSIGNAL);
        
        if (sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Socket buffer full, try again
                continue;
            }
            Utils::log("Error: send() failed: " + std::string(strerror(errno)));
            return false;
        }
        
        if (sent == 0) {
            Utils::log("Error: Connection closed by peer during send.");
            return false;
        }
        
        total_sent += sent;
    }

    return true;
}

std::string TCPSocket::receive() {
    return receive(4096); // Default buffer size
}

bool TCPSocket::isConnected() const {
    return sockfd_ != -1;
}

std::string TCPSocket::receive(size_t max_size) {
    if (sockfd_ < 0) {
        Utils::log("Error: socket is not open.");
        return "";
    }

    char* buffer = new char[max_size];
    
    ssize_t n = ::recv(sockfd_, buffer, max_size, 0);
    
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No data available right now
            delete[] buffer;
            return "";
        }
        Utils::log("Error: recv() failed: " + std::string(strerror(errno)));
        delete[] buffer;
        return "";
    }
    
    if (n == 0) {
        Utils::log("Connection closed by peer.");
        delete[] buffer;
        return "";
    }

    std::string result(buffer, n);
    delete[] buffer;
    return result;
}

std::string TCPSocket::receiveWithTimeout(int timeout_seconds, size_t max_size) {
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

std::string TCPSocket::receiveUntil(const std::string& delimiter, size_t max_size) {
    if (sockfd_ < 0) {
        Utils::log("Error: socket is not open.");
        return "";
    }

    std::string result;
    char buffer[1024];
    
    while (result.size() < max_size) {
        ssize_t n = ::recv(sockfd_, buffer, sizeof(buffer), 0);
        
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            Utils::log("Error: recv() failed: " + std::string(strerror(errno)));
            break;
        }
        
        if (n == 0) {
            Utils::log("Connection closed by peer.");
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

int TCPSocket::getSocketFd() const {
    return sockfd_;
}

bool TCPSocket::setSocketOption(int level, int optname, const void* optval, socklen_t optlen) {
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

bool TCPSocket::setKeepAlive(bool enable) {
    int keepalive = enable ? 1 : 0;
    return setSocketOption(SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));
}

bool TCPSocket::setReceiveTimeout(int seconds) {
    struct timeval timeout;
    timeout.tv_sec = seconds;
    timeout.tv_usec = 0;
    return setSocketOption(SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}

bool TCPSocket::setSendTimeout(int seconds) {
    struct timeval timeout;
    timeout.tv_sec = seconds;
    timeout.tv_usec = 0;
    return setSocketOption(SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
}

std::string TCPSocket::getAddress() const {
    return address_;
}

int TCPSocket::getPort() const {
    return port_;
}
