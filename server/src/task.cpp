#include "task.hpp"

void Task::receive(void *buffer, size_t size)
{
    m_recv_size = size;
    m_recv_buffer = buffer;

    m_sock.async_read_some(asio::buffer(buffer, size), std::bind(&Task::on_receive_internal, this, 
        std::placeholders::_1, std::placeholders::_2));
}

void Task::on_receive_internal(const asio::error_code& ec, size_t bytes)
{
    m_recv_size -= bytes;

    if (m_recv_size)
    {
        receive(m_recv_buffer, m_recv_size);
    }
    else 
    {
        on_receive();
    }
}

