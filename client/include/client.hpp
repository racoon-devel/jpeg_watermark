#pragma once

#include <string>

#include "asio.hpp"

#include "image.hpp"
#include "timer.hpp"
#include "protocol.hpp"

/**
 * @class Client класс для выполнения команд на сервере обработки
 * изображений
 */
class Client final
{
public:
	/**
	 * @struct Settings настройки клиента
	 */
	struct Settings
	{
		//! IP-адрес сервера
		std::string address{"127.0.0.1"};

		//! порт сервера
		uint port{9001};

		//! интервал переподключения, если сервер занят
		uint timeout_sec{5};
	};

	/**
	 * Ctor
	 * @param settings настройки подключения к серверу
	 */
	explicit Client(Settings settings);

	/**
	 * Выполнить запрос к серверу
	 * @tparam Request Тип запроса (должен иметь методы payload_type() и serialize()
	 * @param req Запрос
	 */
	template< class Request >
	void execute(Request req)
	{
		request(Request::payload_type(), req.serialize());
	}

	//! идентификатор клиента
	uint id() const { return m_id; }

	//! закончилось ли успехом обработка изображения
	bool is_success() const { return m_success; }

	//! результирующее изображение
	const Image& image() const { return m_result_image; }

private:
	const Settings m_settings;
	const uint     m_id;

	asio::ip::tcp::socket m_socket;
	DeadlineTimer         m_timer;

	std::vector< uint8_t > m_send_buffer;
	std::vector< uint8_t > m_recv_buffer;

	Image m_result_image;
	bool  m_success{false};

	void request(proto::PayloadType            type,
				 const std::vector< uint8_t >& payload);

	void send_request();
	void on_receive(const asio::error_code& ec, size_t bytes);
	void on_receive_image(const asio::error_code& ec, size_t bytes);
};