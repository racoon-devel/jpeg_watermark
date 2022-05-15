#pragma once

#include <string>
#include <list>

#include "asio.hpp"
#include "image_processor.hpp"
#include "session.hpp"

class Server final
{
public:
	Server(const std::string& addr, int port, uint max_tasks)
		: m_addr(addr), m_port(port), m_max_tasks(max_tasks), m_acceptor(m_io),
		  m_sock(m_io), m_timer(m_io), m_proc(m_io, max_tasks)
	{
	}

	Server()              = delete;
	Server(const Server&) = delete;
	void operator=(const Server&) = delete;

	int Run();

private:
	const uint m_clear_timeout =
		1300; // миллисекунды для таймера очистки неиспользуемых соединений
	using SessionPtr = std::shared_ptr< Session >;

	std::string m_addr;
	int         m_port;
	uint        m_max_tasks;

	asio::io_service                                        m_io;
	asio::ip::tcp::acceptor                                 m_acceptor;
	asio::ip::tcp::socket                                   m_sock;
	asio::basic_waitable_timer< std::chrono::steady_clock > m_timer;

	std::list< SessionPtr > m_sessions;

	ImageProcessor m_proc;

	void shutdown();

	void on_stop(const asio::error_code& ec, int signal_number);
	void on_accept(const asio::error_code& ec);
	void on_tick(const asio::error_code& ec);
};