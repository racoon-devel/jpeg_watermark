#pragma once

#include <string>
#include <memory>

namespace detail
{
struct Service;
}

/**
 * @class Service "фасад" сервиса обработки изображений
 */
class Service final
{
public:
	/**
	 * @struct Settings настройки сервиса обработки изображений
	 */
	struct Settings
	{
		//! IP-адрес для биндинга TCP-сервера
		std::string address = "127.0.0.1";

		//! порт TCP-сервера
		uint port = 9001;

		//! ограничение на макс. кол-во задач обработки изображений
		uint max_jobs = 0;
	};

	/**
	 * Ctor
	 * @param settings настройки сервиса
	 */
	explicit Service(const Settings& settings);

	/**
	 * Запуск сервиса
	 * @throw std::exception при любых ошибках
	 */
	void run();

	~Service();

private:
	std::unique_ptr< detail::Service > m_impl;
};