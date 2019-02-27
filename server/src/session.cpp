#include "session.hpp"
#include <iostream>

void Session::receive(uint8_t *buffer, size_t size)
{
    m_recv_size = size;
    m_recv_buffer = buffer;

    m_sock.async_read_some(asio::buffer(buffer, size), std::bind(&Session::on_receive_internal, this, 
        std::placeholders::_1, std::placeholders::_2));
}

void Session::send(const uint8_t *buffer, size_t size)
{
    m_send_size = size;
    m_send_buffer = buffer;

    m_sock.async_write_some(asio::buffer(buffer, size), std::bind(&Session::on_sent_internal, this, 
        std::placeholders::_1, std::placeholders::_2));
}

void Session::on_receive_internal(const asio::error_code& ec, size_t bytes)
{
    if (ec) 
    {
        std::cerr << "Read error: " << ec.message() << std::endl;
        on_error();
        return ;
    }
    
    m_recv_size -= bytes;

    m_recv_size ? receive(m_recv_buffer + bytes, m_recv_size) : on_receive();
}

void Session::on_sent_internal(const asio::error_code& ec, size_t bytes)
{
    if (ec) 
    {
        std::cerr << "Write error: " << ec.message() << std::endl;
        on_error();
        return ;
    }
    
    m_send_size -= bytes;

    m_send_size ? send(m_send_buffer + bytes, m_send_size) : on_sent();
}

void ProtoSession::on_receive()
{
    switch (m_state)
    {
        case State::kReadHeader:
            std::cerr << "New request { text size: " << m_header.text_size << " image size: " << m_header.image_size << " }" << std::endl;
            // TODO: Неплохо бы сделать ограничения на размер текста и картинки
            m_text_buffer.resize(m_header.text_size);
            m_image_buffer.resize(m_header.image_size);

            m_state = State::kReadText;
            receive(&m_text_buffer[0], m_header.text_size);
            break;

        case State::kReadText:
            std::cerr << "Text received" << std::endl;
            m_state = State::kReadImage;
            receive(&m_image_buffer[0], m_header.image_size);
            break;

        case State::kReadImage:
            std::cerr << "Image received" << std::endl;
            m_state = State::kProcessing;

            if (!m_proc.Enqueue(Task(shared_from_this(), std::string(m_text_buffer.begin(), m_text_buffer.end()), std::move(m_image_buffer))))
            {
                send_header(kStatusBusy);
            }

            break;

        default:
            return;
    }
}            

void ProtoSession::on_error()
{
    send_header(kStatusError);
}

void ProtoSession::on_sent()
{
    if (m_state == State::kWriteHeader && m_have_response)
    {
        std::cerr << "Header sent" << std::endl;

        m_state = State::kWriteResult;
        send(&m_image_buffer[0], m_image_buffer.size());
    }
    else 
    {
        std::cerr << "Session done" << std::endl;
    }
}

// TODO: std::move
void ProtoSession::complete(std::vector<uint8_t> image)
{
    m_image_buffer = image;

    send_header(kStatusOK);

    std::cerr << "Task done" << std::endl;
}

void ProtoSession::send_header(StatusCode code)
{
    m_state = State::kWriteHeader;
    m_response.code = code; 
    m_response.image_size = code == kStatusOK ? m_image_buffer.size() : 0;
    m_have_response = code == kStatusOK;
    
    send((uint8_t*) &m_response, sizeof(m_response));    
}

