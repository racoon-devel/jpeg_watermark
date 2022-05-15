#include "client.hpp"

#include "easylogging++.h"

static uint max_client_id;

ostream& operator<<(ostream& os, ProtoClient * client)
{
    os << "[ Client: " << client->id() << " ] ";
    return os;
}

ProtoClient::ProtoClient(asio::io_service& io, const Image& image, const string& server_addr, int server_port, const string& text, uint timeout)
    : m_io(io), m_image(image),
    m_addr(server_addr), m_port(server_port),
    m_text(text), m_reconnect_time(timeout), 
    m_sock(io), m_timer(io), m_id(max_client_id++),
    m_success(false)
{

}

void ProtoClient::Run()
{
    send_request();
}

void ProtoClient::send_request()
{
    try
    {
        asio::ip::tcp::endpoint ep(asio::ip::address::from_string(m_addr), m_port);
        m_sock.open(asio::ip::tcp::v4());
        m_sock.connect(ep);

        m_buffer.resize(sizeof(ProtoDataHeader) + m_text.size() + m_image.size());
        
        auto ptr = &m_buffer[0];

        ProtoDataHeader * header = (ProtoDataHeader *) ptr;
        header->sign = PROTO_VALID_SIGN;
        header->text_size = m_text.size();
        header->image_size = m_image.size();

        ptr += sizeof(ProtoDataHeader);
        memcpy(ptr, m_text.data(), m_text.size());

        ptr += m_text.size();
        memcpy(ptr, m_image.data(), m_image.size());

        asio::async_write(m_sock, asio::buffer(&m_buffer[0], m_buffer.size()),
        [this] (const asio::error_code& ec, size_t bytes)
        {
            asio::async_read(this->m_sock, asio::buffer((uint8_t *) &this->m_header, sizeof(this->m_header)),
                std::bind(&ProtoClient::on_receive, this, std::placeholders::_1, std::placeholders::_2));
        });
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << this << "Connect failed. " << e.what();
    }
}

void ProtoClient::on_receive(const asio::error_code& ec, size_t bytes)
{
    if (ec)
    {
        LOG(ERROR) << this << "Read error. " << ec.message();
        return ;
    }

    switch (m_header.code)
    {
        case kStatusBusy:
            LOG(WARNING) << this << "Server busy. Reconnect after timeout";

            m_timer.expires_from_now(std::chrono::seconds(m_reconnect_time));

            m_sock.close();
            
            m_timer.async_wait([this] (const asio::error_code& ec)
            {
                if (ec) return ;

                LOG(INFO) << this << "Reconnecting...";

                this->send_request();
            });

            break;
        
        case kStatusError:
            LOG(ERROR) << this << "Server reply: error";
            break;

        case kStatusLimit:
            LOG(ERROR) << this << "Server reply: image or text size limit reached";
            break;    

        case kStatusOK:
            LOG(INFO) << this << "Server reply: OK";
            
            m_result_image.resize(m_header.image_size);

            asio::async_read(m_sock, asio::buffer(&m_result_image[0], m_result_image.size()),
                std::bind(&ProtoClient::on_receive_image, this, std::placeholders::_1, std::placeholders::_2));
            break;

        default:
            return ;

    }
}

void ProtoClient::on_receive_image(const asio::error_code& ec, size_t bytes)
{
    LOG(INFO) << this << "Image received";

    m_success = true;
}