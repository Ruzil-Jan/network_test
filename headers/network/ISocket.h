#pragma once

#include <string>

class ISocket {
public:
    virtual ~ISocket() = default;
    virtual bool open() = 0;
    virtual void close() = 0;
    virtual bool send(const std::string& data) = 0;
    virtual std::string receive() = 0;
};

