#pragma once

#include <string>

#include "asio.hpp"

class Server final
{
public:
    Server(const std::string& addr, int port, uint max_tasks)
        : m_addr(addr), m_port(port), m_max_tasks(max_tasks)
    {}

    Server() = delete;
    Server(const Server&) = delete;

private:
    std::string m_addr;
    int m_port;
    uint m_max_tasks;

    asio::io_service m_io;
};