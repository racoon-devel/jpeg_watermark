#include "server.hpp"

#include "easylogging++.h"

#include "io_service.hpp"

Server::Server(std::string addr, uint port)
	: m_addr(std::move(addr)), m_port(port), m_acceptor(IoService::get()),
	  m_sock(IoService::get())
{
}

void Server::run(Server::Handler&& handler)
{
	m_handler = std::move(handler);

	if (!m_acceptor.is_open())
	{
		// биндимся на порт
		asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string(m_addr),
										 m_port);
		m_acceptor.open(endpoint.protocol());
		m_acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
		m_acceptor.bind(endpoint);
		m_acceptor.listen();

		m_acceptor.async_accept(
			m_sock, std::bind(&Server::on_accept, this, std::placeholders::_1));

		LOG(INFO) << "Server started [ addr = " << m_addr
				  << ", port = " << m_port << " ]";
	}
}

void Server::on_accept(const asio::error_code& ec)
{
	if (ec)
	{
		LOG(ERROR) << "Accept failed: " << ec.message();
		return;
	}

	try
	{
		const auto endpoint = m_sock.remote_endpoint();
		LOG(DEBUG) << "Client accepted: " << endpoint.address() << ":"
				   << endpoint.port();
	}
	catch (const asio::system_error& e)
	{
		LOG(WARNING) << e.what();
	}

	m_handler(std::move(m_sock));

	m_acceptor.async_accept(
		m_sock, std::bind(&Server::on_accept, this, std::placeholders::_1));
}
