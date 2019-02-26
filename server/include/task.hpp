#pragma once

#include "asio.hpp"
#include <vector>

#include "protocol.hpp"

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

    virtual void start() = 0;

protected:
    /* Интерфейс для сетевого взаимодействия для подклассов */
    void receive(uint8_t *buffer, size_t size);         // Запрос на получение порции данных размера size в буфер buffer
    void send(const uint8_t *buffer, size_t size);      // Запрос на отправку данных

    /* Callbacks, которые нужно переопределить в подклассах */
    virtual void on_receive() = 0;
    virtual void on_sent() = 0;
    virtual void on_error() = 0;

private:
    void on_receive_internal(const asio::error_code& ec, size_t bytes);
    void on_sent_internal(const asio::error_code& ec, size_t bytes);

    asio::ip::tcp::socket m_sock;
    
    size_t m_recv_size, m_send_size;
    uint8_t *m_recv_buffer;
    const uint8_t *m_send_buffer;
};

// Задача накладывания текста на изображение
class WatermarkTask : public Task
{
public:
    explicit WatermarkTask(asio::ip::tcp::socket&& sock)
        : Task(std::move(sock)),
        m_state(State::kReadHeader),
        m_have_response(false)
    {}

    virtual void start() override { receive((uint8_t*) &m_header, sizeof(m_header)); }

protected:
    virtual void on_receive() override;
    virtual void on_sent() override;
    virtual void on_error() override;

private:

    enum class State { kReadHeader, kReadText, kReadImage, kProcessing, kWriteHeader, kWriteResult };
    State m_state;

    ProtoDataHeader m_header;
    std::vector<uint8_t> m_text_buffer;
    std::vector<uint8_t> m_image_buffer;

    ProtoResponseHeader m_response;
    std::vector<uint8_t> m_output;

    bool m_have_response;
};