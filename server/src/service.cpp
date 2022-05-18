#include "service.hpp"

#include "easylogging++.h"

#include "server.hpp"
#include "session_manager.hpp"
#include "io_service.hpp"
#include "worker_pool.hpp"

namespace detail
{
struct Service
{
	explicit Service(const ::Service::Settings& settings)
		: m_server(settings.address, settings.port), m_pool(settings.max_jobs)
	{
	}

	void run()
	{
		auto& io = IoService::get();

		// завершение сервиса по сигналу
		asio::signal_set signals(io, SIGINT, SIGTERM);
		signals.async_wait([this](auto& ec, int) { on_shutdown(ec); });

		// запускаем менеджер соединений
		m_mgr.run();

		// запускаем TCP-сервер
		m_server.run([this](auto&& socket)
					 { on_client(std::forward< decltype(socket) >(socket)); });

		// запускаем пул потоков
		m_pool.run();

		io.run();
	}

private:
	Server         m_server;
	SessionManager m_mgr;
	WorkerPool     m_pool;

	void on_shutdown(const asio::error_code&)
	{
		// останавливаем весь IO
		IoService::get().stop();

		// останавливаем все задачи
		m_pool.shutdown();
	}

	void on_client(asio::ip::tcp::socket&& socket) noexcept
	{
		try
		{
			m_mgr.add_session(m_pool, std::move(socket));
		}
		catch (const std::exception& e)
		{
			LOG(ERROR) << "Run client session failed: " << e.what();
		}
	}
};

} // namespace detail

Service::Service(const Service::Settings& settings)
	: m_impl(std::make_unique< detail::Service >(settings))
{
}

void Service::run()
{
	m_impl->run();
}

// для pimpl
Service::~Service() = default;
