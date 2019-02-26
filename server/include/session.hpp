#pragma once

#include "asio.hpp"
#include <vector>

#include "protocol.hpp"
#include "image_processor.hpp"

// Общий класс - просто на случай, если будут задачи другого типа
class Session
{
public:
    Session(ImageProcessor& proc, asio::ip::tcp::socket&& sock)
        : m_proc(proc),
        m_sock(std::move(sock)),
        m_recv_size(0), m_send_size(0),
        m_recv_buffer(nullptr), m_send_buffer(nullptr)
    {}

    Session(const Session&) = delete;
    void operator=(const Session&) = delete;
    
    Session(Session&& other)
        : m_proc(other.m_proc),
        m_sock(std::move(other.m_sock))
    {}

    virtual ~Session() { m_sock.close(); }

    virtual void start() = 0;

protected:
    ImageProcessor& m_proc;

    /* Интерфейс для сетевого взаимодействия для подклассов */
    void receive(uint8_t *buffer, size_t size);         // Запрос на получение порции данных размера size в буфер buffer
    void send(const uint8_t *buffer, size_t size);      // Запрос на отправку данных

    /* Callbacks ввода/вывода, которые нужно переопределить в подклассах */
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

// Прием и отправка изображений по самопальному "протоколу"
class ProtoSession : public Session
{
public:
    explicit ProtoSession(ImageProcessor &pool, asio::ip::tcp::socket&& sock)
        : Session(pool, std::move(sock)),
        m_state(State::kReadHeader),
        m_have_response(false)
    {}

    virtual void start() override { receive((uint8_t*) &m_header, sizeof(m_header)); }

    void complete(std::vector<uint8_t> image);

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

    bool m_have_response;

    void send_header(StatusCode code);
};