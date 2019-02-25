#pragma once

#include "asio.hpp"

class Task
{
public:
    explicit Task(asio::ip::tcp::socket&& sock)
        : m_sock(std::move(sock)),
        m_recv_size(0), m_send_size(0),
        m_recv_buffer(nullptr), m_send_buffer(nullptr)
    {}

    Task(const Task&) = delete;
    
    Task(Task&& other)
        : m_sock(std::move(other.m_sock))
    {}

    virtual ~Task() { m_sock.close(); }

    virtual void execute() = 0;

protected:
    /* Интерфейс для сетевого взаимодействия для подклассов */
    void receive(void *buffer, size_t size); // Запрос на получение порции данных размера size в буфер buffer
    void send(void *buffer, size_t size);    // Запрос на отправку данных

    virtual void on_receive() = 0;

private:
    void on_receive_internal(const asio::error_code& ec, size_t bytes);
    
    asio::ip::tcp::socket m_sock;
    
    size_t m_recv_size, m_send_size;
    void *m_recv_buffer, *m_send_buffer;
};

class WatermarkTask : public Task
{
public:
    explicit WatermarkTask(asio::ip::tcp::socket&& sock)
        : Task(std::move(sock))
    {}

    virtual void execute() override;
};