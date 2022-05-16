#pragma once

#include <timer.hpp>
#include "asio.hpp"

#include "task/invoker.hpp"

/**
 * @class Session базовый класс обработки сессии с клиентом
 */
class Session
{
public:
	/**
	 * Ctor
	 * @param invoker Интерфейс для выполнения задач
	 * @param socket Сокет соединения
	 */
	Session(IInvoker& invoker, asio::ip::tcp::socket&& socket);

	/**
	 * Запуск взаимодействия с клиентом
	 * @throw any exception
	 */
	virtual void run() = 0;

	//! флаг, что сессия завершилась
	bool is_done() const noexcept { return m_done; }

	//! ID сессии для логирования
	std::string identity() const noexcept { return m_identity; }

	virtual ~Session() = default;

protected:
	IInvoker& m_invoker;

	/* операции ввода/вывода для подклассов */
	void receive(std::vector< uint8_t >& buffer);
	void send(const std::vector< uint8_t >& buffer);
	void done();

	/* Callbacks ввода/вывода, которые нужно переопределить в подклассах */
	virtual void on_received() = 0;
	virtual void on_sent()     = 0;

private:
	const uint            m_io_timeout_ms = 10000;
	std::string           m_identity;
	asio::ip::tcp::socket m_socket;
	DeadlineTimer         m_timer;

	bool m_done{false};

	void on_received_internal(const asio::error_code& ec, size_t bytes);
	void on_sent_internal(const asio::error_code& ec, size_t bytes);
	void timer_restart();
};