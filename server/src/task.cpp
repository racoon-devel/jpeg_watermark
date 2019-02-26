#include "task.hpp"
#include <iostream>

void Task::receive(uint8_t *buffer, size_t size)
{
    m_recv_size = size;
    m_recv_buffer = buffer;

    m_sock.async_read_some(asio::buffer(buffer, size), std::bind(&Task::on_receive_internal, this, 
        std::placeholders::_1, std::placeholders::_2));
}

void Task::send(const uint8_t *buffer, size_t size)
{
    m_send_size = size;
    m_send_buffer = buffer;

    m_sock.async_write_some(asio::buffer(buffer, size), std::bind(&Task::on_sent_internal, this, 
        std::placeholders::_1, std::placeholders::_2));
}

void Task::on_receive_internal(const asio::error_code& ec, size_t bytes)
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

void Task::on_sent_internal(const asio::error_code& ec, size_t bytes)
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

void WatermarkTask::on_receive()
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
            m_state = State::kWriteHeader;

            /* XXX */
            m_response.code = kStatusOK; m_response.image_size = 1000;
            m_have_response = true;
            send((uint8_t*) &m_response, sizeof(m_response));
            break;

        default:
            return;
    }
}

void WatermarkTask::on_error()
{
    m_response.image_size = 0;
    m_response.code = kStatusError;
    m_state = State::kWriteHeader;
    m_have_response = false;

    send((uint8_t*) &m_response, sizeof(m_response));
}

void WatermarkTask::on_sent()
{
    if (m_state == State::kWriteHeader && m_have_response)
    {
        std::cerr << "Header sent" << std::endl;

        m_state = State::kWriteResult;
        m_output.resize(1000, 0);
        send(&m_output[0], m_output.size());
    }
    else 
    {
        std::cerr << "Task done" << std::endl;
    }
}

