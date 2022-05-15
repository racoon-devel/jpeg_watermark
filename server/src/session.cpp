#include "session.hpp"
#include <iostream>

#include "easylogging++.h"

std::ostream& operator<<(std::ostream& os, const Session * session)
{
    os << "[Session:" << session->identify() << "] ";
    return os;
}


Session::Session(ImageProcessor& proc, asio::ip::tcp::socket&& sock)
    : m_proc(proc),
    m_sock(std::move(sock)),
    m_timer(m_sock.get_io_service()),
    m_done(false)
{
    try
    {
        m_identify = m_sock.remote_endpoint().address().to_string() + ":"
            + std::to_string(m_sock.remote_endpoint().port());
    }
    catch(const std::exception&)
    {
        
    }
}

void Session::receive(uint8_t *buffer, size_t size)
{
    timer_restart();
    
    asio::async_read(m_sock, asio::buffer(buffer, size), std::bind(&Session::on_receive_internal, this, 
        std::placeholders::_1, std::placeholders::_2));
}

void Session::send(const uint8_t *buffer, size_t size)
{
    timer_restart();

    asio::async_write(m_sock, asio::buffer(buffer, size), std::bind(&Session::on_sent_internal, this, 
        std::placeholders::_1, std::placeholders::_2));
}

void Session::on_receive_internal(const asio::error_code& ec, size_t bytes)
{   
    (void) bytes;

    m_timer.cancel();
    
    if (ec) 
    {
        LOG(ERROR) << this << "Read error: " << ec.message();
        on_error();
        return ;
    }

    LOG(DEBUG) << this << "Received " << bytes << " bytes";
    
    on_receive();
}

void Session::on_sent_internal(const asio::error_code& ec, size_t bytes)
{
    (void) bytes;

    m_timer.cancel();

    if (ec) 
    {
        LOG(ERROR) << this << "Write error: " << ec.message();
        on_error();
        return ;
    }
    
    on_sent();
}

void Session::timer_restart()
{
    m_timer.cancel();
    m_timer.expires_from_now(std::chrono::seconds(m_io_timeout));
    
    m_timer.async_wait([this](const asio::error_code& ec)
    {
        if (!ec) {
            LOG(ERROR) << this <<  "Connection timeout";
            this->done();
        }
    });
}

void ProtoSession::on_receive()
{
    switch (m_state)
    {
        case State::kReadHeader:
            LOG(DEBUG) << this << "New request { text size: " << m_header.text_size << " image size: " << m_header.image_size << " }";
            
            if (m_header.text_size > m_max_text_size || m_header.image_size > m_max_image_size)
            {
                LOG(ERROR) << this << "Data size limit reached";
                send_header(StatusCode::kStatusLimit);
                return ;
            }

            m_text_buffer.resize(m_header.text_size);
            m_image_buffer.resize(m_header.image_size);

            m_state = State::kReadText;
            receive(&m_text_buffer[0], m_header.text_size);
            break;

        case State::kReadText:
            LOG(DEBUG) << this << "Text received";
            m_state = State::kReadImage;
            receive(&m_image_buffer[0], m_header.image_size);
            break;

        case State::kReadImage:
            LOG(DEBUG) << this << "Image received";
            m_state = State::kProcessing;

            if (!m_proc.Enqueue(Task(shared_from_this(), std::string(m_text_buffer.begin(), m_text_buffer.end()), std::move(m_image_buffer))))
            {
                LOG(WARNING) << this << "Request rejected, max jobs count reached";
                send_header(kStatusBusy);
            }

            break;

        default:
            return;
    }
}            

void ProtoSession::on_error()
{
    /* Если еще не было ошибок ввода/вывода - то сообщаем об ошибке */
    if (!m_have_errors)
    {
        m_have_errors = true;
        send_header(kStatusError);
        return;
    }

    /* Не смогли отправить сообщение об ощибке - отключаемся */
    done();
}

void ProtoSession::on_sent()
{
    if (m_state == State::kWriteHeader && m_have_response)
    {
        m_state = State::kWriteResult;
        send(&m_image_buffer[0], m_image_buffer.size());
    }
    else 
    {
        LOG(INFO) << this << "Session done";
        done();
    }
}

void ProtoSession::complete(bool is_success, const std::vector<uint8_t> image)
{
    if (is_success)
    {
        m_image_buffer = image;
        send_header(kStatusOK);
    }
    else
    {
        send_header(kStatusError);
    }
    

    LOG(INFO) << this << "Image job done";
}

void ProtoSession::send_header(StatusCode code)
{
    m_state = State::kWriteHeader;
    m_response.code = code; 
    m_response.image_size = code == kStatusOK ? m_image_buffer.size() : 0;
    m_have_response = code == kStatusOK;
    
    send(reinterpret_cast<uint8_t*>(&m_response), sizeof(m_response));    
}

