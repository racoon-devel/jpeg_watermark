#include <session_proto.hpp>
#include "session_manager.hpp"

#include "session.hpp"
#include "io_service.hpp"
#include "easylogging++.h"

SessionManager::SessionManager() : m_timer(IoService::get())
{
}

void SessionManager::run()
{
	/* Таймер нужен для очистки завершенных сессий - не очень красиво, но здесь
	 * большой нагрузки не ожидается */
	m_timer.expires_from_now(std::chrono::milliseconds(m_clear_timeout_ms));
	m_timer.async_wait(
		std::bind(&SessionManager::on_tick, this, std::placeholders::_1));
}

void SessionManager::add_session(IInvoker&               invoker,
								 asio::ip::tcp::socket&& socket)
{
	auto session_ptr =
		std::make_shared< ProtoSession >(invoker, std::move(socket));

	m_sessions.push_back(session_ptr);

	session_ptr->run();
}

void SessionManager::on_tick(const asio::error_code& ec)
{
	if (ec)
	{
		LOG(WARNING) << "Timer error: " << ec.message();
		return;
	}

	size_t total = m_sessions.size();

	m_sessions.remove_if([](auto& session) { return session->is_done(); });

	m_timer.expires_from_now(std::chrono::milliseconds(m_clear_timeout_ms));
	m_timer.async_wait(
		std::bind(&SessionManager::on_tick, this, std::placeholders::_1));

	total -= m_sessions.size();

	if (total)
	{
		LOG(DEBUG) << total << " sessions released";
	}
}
