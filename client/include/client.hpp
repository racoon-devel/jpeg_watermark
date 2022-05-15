#include <string>
#include <vector>
#include <timer.hpp>

#include "protocol.hpp"
#include "image.hpp"
#include "asio.hpp"

/**
 * @class ProtoClient клиент для сервера обработки изображений по
 * самопальному протоколу
 */
class ProtoClient final
{
public:
	/**
	 * @struct Settings настройки клиента
	 */
	struct Settings
	{
		//! изображение для обработки
		Image image;

		//! IP-адрес сервера
		std::string address{"127.0.0.1"};

		//! порт сервера
		uint port{9001};

		//! текст для печати
		std::string text;

		//! интервал переподключения, если сервер занят
		uint timeout_sec{5};
	};

	explicit ProtoClient(Settings settings);

	ProtoClient(const ProtoClient&)             = delete;
	ProtoClient& operator==(const ProtoClient&) = delete;

	//! запуск клиента
	void run();

	//! идентификатор клиента
	uint id() const { return m_id; }

	//! закончилось ли успехом обработка изображения
	bool         is_success() const { return m_success; }

	//! результирующее изображение
	const Image& image() const { return m_result_image; }

private:
	Settings m_settings;

	asio::ip::tcp::socket m_socket;
	DeadlineTimer         m_timer;

	uint  m_id;
	Image m_buffer;

	ProtoResponseHeader m_header;
	Image               m_result_image;
	bool                m_success{false};

	void send_request();
	void on_receive(const asio::error_code& ec, size_t bytes);
	void on_receive_image(const asio::error_code& ec, size_t bytes);
};