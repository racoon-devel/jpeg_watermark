#include "session/session.hpp"
#include <iostream>
#include <task/watermark_task.hpp>
#include <io_service.hpp>

#include "easylogging++.h"

std::ostream& operator<<(std::ostream& os, const Session* session)
{
	os << "[Session:" << session->identity() << "] ";
	return os;
}

Session::Session(IInvoker& invoker, asio::ip::tcp::socket&& sock)
	: m_invoker(invoker), m_socket(std::move(sock)), m_timer(IoService::get()),
	  m_done(false)
{
	try
	{
		const auto endpoint = m_socket.remote_endpoint();
		m_identity          = endpoint.address().to_string() + ":"
			+ std::to_string(endpoint.port());
	}
	catch (const std::exception&)
	{
	}

}

void Session::receive(std::vector< uint8_t >& buffer)
{
	timer_restart();

	asio::async_read(m_socket, asio::buffer(buffer.data(), buffer.size()),
					 std::bind(&Session::on_received_internal, this,
							   std::placeholders::_1, std::placeholders::_2));
}

void Session::send(const std::vector< uint8_t >& buffer)
{
	timer_restart();

	asio::async_write(m_socket,
					  asio::const_buffer(buffer.data(), buffer.size()),
					  std::bind(&Session::on_sent_internal, this,
								std::placeholders::_1, std::placeholders::_2));
}

void Session::on_received_internal(const asio::error_code& ec, size_t bytes)
{
	(void) bytes;

	m_timer.cancel();

	if (ec)
	{
		LOG(ERROR) << this << "Read error: " << ec.message();
		done();
		return;
	}

	LOG(DEBUG) << this << "Received " << bytes << " bytes";

	try
	{
		on_received();
	}
	catch (const std::exception& e)
	{
		LOG(ERROR) << this << "Session error: " << e.what();
		done();
	}
}

void Session::on_sent_internal(const asio::error_code& ec, size_t bytes)
{
	(void) bytes;

	m_timer.cancel();

	if (ec)
	{
		LOG(ERROR) << this << "Write error: " << ec.message();
		done();
		return;
	}

	try
	{
		on_sent();
	}
	catch (const std::exception& e)
	{
		LOG(ERROR) << this << "Session error: " << e.what();
		done();
	}
}

void Session::timer_restart()
{
	m_timer.cancel();
	m_timer.expires_from_now(std::chrono::milliseconds(m_io_timeout_ms));

	m_timer.async_wait(
		[this](const asio::error_code& ec)
		{
			if (!ec)
			{
				LOG(ERROR) << this << "I/O timeout";
				done();
			}
		});
}

void Session::done()
{
	m_done = true;
	m_socket.close();
}
