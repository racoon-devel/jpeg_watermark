#pragma once

#include <list>

#include "asio.hpp"
#include "timer.hpp"
#include "task/invoker.hpp"

class Session;

/**
 * @class SessionManager управляет клиентскими подключениями
 */
class SessionManager
{
public:
	SessionManager();

	/**
	 * Запускаем необходимое поведение в петле событий
	 * @throw asio::system_error
	 */
	void run();

	/**
	 * Добавить новую сессию обмена сообщениями с клиентом
	 * @param invoker Сущность для выполнения задач
	 * @param socket Сокет
	 * @throw asio::system_error
	 */
	void add_session(IInvoker& invoker, asio::ip::tcp::socket&& socket);

private:
	//! интервал таймера очистки неиспользуемых соединений
	const uint m_clear_timeout_ms = 1300;

	std::list< std::shared_ptr< Session > > m_sessions;
	DeadlineTimer                           m_timer;

	void on_tick(const asio::error_code& ec);
};