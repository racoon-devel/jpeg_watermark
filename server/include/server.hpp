#pragma once

#include <string>

#include "asio.hpp"
#include "threadpool.hpp"

class Server final
{
public:
    Server(const std::string& addr, int port, uint max_tasks)
        : m_addr(addr), m_port(port), m_max_tasks(max_tasks),
        m_acceptor(m_io), m_sock(m_io)
    {}

    Server() = delete;
    Server(const Server&) = delete;

    int Run();

private:
    std::string m_addr;
    int m_port;
    uint m_max_tasks;

    asio::io_service m_io;
    asio::ip::tcp::acceptor m_acceptor;
    asio::ip::tcp::socket m_sock;

    ThreadPool m_threads;

    void on_stop(const asio::error_code& ec, int signal_number);
    void on_accept(const asio::error_code& ec);
};