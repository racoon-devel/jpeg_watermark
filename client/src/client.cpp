#include <io_service.hpp>
#include <utility>
#include "../include/client.hpp"

#include "easylogging++.h"

static uint max_client_id;

std::ostream& operator<<(std::ostream& os, ProtoClient* client)
{
	os << "[ Client: " << client->id() << " ] ";
	return os;
}

ProtoClient::ProtoClient(ProtoClient::Settings settings)
	: m_settings(std::move(settings)), m_socket(IoService::get()),
	  m_timer(IoService::get()), m_id(max_client_id++)
{
}

void ProtoClient::run()
{
	send_request();
}

void ProtoClient::send_request()
{
	try
	{
		asio::ip::tcp::endpoint ep(
			asio::ip::address::from_string(m_settings.address),
			m_settings.port);
		m_socket.open(asio::ip::tcp::v4());
		m_socket.connect(ep);

		m_buffer.resize(sizeof(ProtoDataHeader) + m_settings.text.size()
						+ m_settings.image.size());

		auto ptr = &m_buffer[0];

		auto header = reinterpret_cast< ProtoDataHeader* >(ptr);
		header->sign            = PROTO_VALID_SIGN;
		header->text_size       = m_settings.text.size();
		header->image_size      = m_settings.image.size();

		ptr += sizeof(ProtoDataHeader);
		memcpy(ptr, m_settings.text.data(), m_settings.text.size());

		ptr += m_settings.text.size();
		memcpy(ptr, m_settings.image.data(), m_settings.image.size());

		asio::async_write(m_socket, asio::buffer(&m_buffer[0], m_buffer.size()),
						  [this](const asio::error_code& ec, size_t bytes)
						  {
							  asio::async_read(
					m_socket,
								  asio::buffer((uint8_t*) &m_header,
											   sizeof(m_header)),
								  std::bind(&ProtoClient::on_receive, this,
											std::placeholders::_1,
											std::placeholders::_2));
						  });
	}
	catch (const std::exception& e)
	{
		LOG(ERROR) << this << "Connect failed: " << e.what();
	}
}

void ProtoClient::on_receive(const asio::error_code& ec, size_t bytes)
{
	if (ec)
	{
		LOG(ERROR) << this << "Read error. " << ec.message();
		return;
	}

	switch (m_header.code)
	{
	case kStatusBusy:
		LOG(WARNING) << this << "Server busy. Reconnect after timeout";

		m_timer.expires_from_now(std::chrono::seconds(m_settings.timeout_sec));

		m_socket.close();

		m_timer.async_wait(
			[this](const asio::error_code& ec)
			{
				if (ec)
					return;

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

		asio::async_read(
			m_socket, asio::buffer(&m_result_image[0], m_result_image.size()),
			std::bind(&ProtoClient::on_receive_image, this,
					  std::placeholders::_1, std::placeholders::_2));
		break;

	default:
		return;
	}
}

void ProtoClient::on_receive_image(const asio::error_code& ec, size_t bytes)
{
	LOG(INFO) << this << "Image received";

	m_success = true;
}