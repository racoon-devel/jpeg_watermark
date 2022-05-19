#include <io_service.hpp>
#include <easylogging++.h>

#include "client.hpp"

static uint max_client_id;

const uint io_timeout_ms = 30000;

std::ostream& operator<<(std::ostream& os, Client* client)
{
	os << "[ Client: " << client->id() << " ] ";
	return os;
}

Client::Client(Client::Settings settings)
	: m_settings(std::move(settings)), m_id(max_client_id++),
	  m_socket(IoService::get()), m_timer(IoService::get())
{
}

void Client::request(proto::PayloadType            type,
					 const std::vector< uint8_t >& payload)
{
	if (type == proto::PayloadType::kResponse)
	{
		throw std::invalid_argument("request cannot has response type id");
	}

	m_send_buffer.resize(sizeof(proto::Header) + payload.size());

	auto header  = reinterpret_cast< proto::Header* >(m_send_buffer.data());
	header->sign = proto::Signature;
	header->type = type;
	header->size = payload.size();

	std::copy(payload.begin(), payload.end(),
			  m_send_buffer.begin() + sizeof(proto::Header));

	send_request();
}

void Client::send_request()
{
	asio::ip::tcp::endpoint endpoint(
		asio::ip::address::from_string(m_settings.address), m_settings.port);
	m_socket.open(asio::ip::tcp::v4());

	restart_timeout();

	m_socket.async_connect(
		endpoint,
		[this](const asio::error_code& ec)
		{
			if (ec)
			{
				LOG(ERROR) << this << "Connect failed: " << ec.message();
				done();
				return;
			}

			restart_timeout();

			asio::async_write(
				m_socket,
				asio::const_buffer(&m_send_buffer[0], m_send_buffer.size()),
				std::bind(&Client::on_sent, this, std::placeholders::_1,
						  std::placeholders::_2));
		});
}

void Client::on_receive(const asio::error_code& ec, size_t bytes)
{
	using namespace proto;

	m_timer.cancel();

	if (ec)
	{
		LOG(ERROR) << this << "Read failed: " << ec.message();
		done();
		return;
	}

	auto header = reinterpret_cast< Header* >(m_recv_buffer.data());
	if (header->sign != Signature || header->type != PayloadType::kResponse
		|| header->size < sizeof(ResponsePayload))
	{
		LOG(ERROR) << this << "Malformed packet received";
		return;
	}

	auto response = reinterpret_cast< ResponsePayload* >(m_recv_buffer.data()
														 + sizeof(Header));

	switch (response->status)
	{
	case Status::kBusy:
		LOG(WARNING) << this << "Server busy. Reconnect after timeout";

		m_socket.close();
		m_timer.expires_from_now(std::chrono::seconds(m_settings.timeout_sec));

		m_timer.async_wait(
			[this](const asio::error_code& ec)
			{
				if (ec)
				{
					LOG(ERROR) << this << "Timer error: " << ec.message();
					return;
				}

				LOG(INFO) << this << "Reconnecting...";

				send_request();
			});

		break;
	case Status::kError:
		LOG(ERROR) << this << "Server reply: error";
		break;

	case Status::kLimit:
		LOG(ERROR) << this << "Server reply: image or text size limit reached";
		break;

	case Status::kOk:
		LOG(INFO) << this << "Server reply: OK";

		m_result_image.resize(response->image_size);

		restart_timeout();

		asio::async_read(
			m_socket, asio::buffer(&m_result_image[0], m_result_image.size()),
			std::bind(&Client::on_receive_image, this, std::placeholders::_1,
					  std::placeholders::_2));
		break;
	}
}

void Client::on_receive_image(const asio::error_code& ec, size_t bytes)
{
	done();

	if (ec)
	{
		LOG(ERROR) << this << "Receive image failed: " << ec.message();
		return;
	}

	LOG(INFO) << this << "Image received";

	m_success = true;
}

void Client::restart_timeout()
{
	m_timer.cancel();
	m_timer.expires_from_now(std::chrono::milliseconds(io_timeout_ms));

	m_timer.async_wait(
		[this](const auto& ec)
		{
			if (ec)
			{
				return;
			}

			LOG(ERROR) << this << "Operation timeout";
			done();
		});
}

void Client::done()
{
	m_timer.cancel();
	m_socket.close();
}

void Client::on_sent(const asio::error_code& ec, size_t bytes)
{
	{
		if (ec)
		{
			LOG(ERROR) << this << "Write failed: " << ec.message();
			done();
			return;
		}

		restart_timeout();

		m_recv_buffer.resize(sizeof(proto::Header)
							 + sizeof(proto::ResponsePayload));

		asio::async_read(
			m_socket, asio::buffer(m_recv_buffer.data(), m_recv_buffer.size()),
			std::bind(&Client::on_receive, this, std::placeholders::_1,
					  std::placeholders::_2));
	}
}
