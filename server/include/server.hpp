#pragma once

#include <string>

#include "asio.hpp"

/**
 * @class Server принимает соединения по TCP и запускает на каждое соединение
 * сессию обработки запросов
 */
class Server final
{
public:
	//! @typedef Handler тип обработчика входящих соединений
	using Handler = std::function< void(asio::ip::tcp::socket&&) noexcept >;

	/**
	 * Ctor
	 * @param addr Адрес
	 * @param port Порт
	 */
	Server(std::string addr, uint port);

	/**
	 * Запуск сервера
	 * @param handler Обработчик входящих подключений
	 * @throw asio::system_error в случае ошибок
	 */
	void run(Handler&& handler);

private:
	const std::string m_addr;
	const uint        m_port;

	Handler m_handler;

	asio::ip::tcp::acceptor m_acceptor;
	asio::ip::tcp::socket   m_sock;

	void on_accept(const asio::error_code& ec);
};