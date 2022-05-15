#pragma once

#include <exception>
#include <memory>
#include <functional>

#include "image.hpp"

class Task;

//! @typedef TaskPtr алиас на шаренный указатель задачи
using TaskPtr = std::shared_ptr< Task >;

/**
 * @class Task базовый класс для команд обработки изображений.
 * Так как команд может быть множество + они ставятся в очередь и могут быть
 * прерываемыми, то имеет смысл использовать паттерн Команда
 */
class Task
{
public:
	using CompleteHandler = std::function< void() noexcept >;

	/**
	 * Ctor
	 * @param handler Обработчик будет вызван после завершения выполнения задачи
	 */
	explicit Task(CompleteHandler&& handler)
		: m_handler(std::move(handler))
	{}

	/**
	 * Выполнить команду
	 */
	void execute() noexcept;

	/**
	 * Получить результат выполнения команды
	 * @return Изображение
	 * @throw Исключение, которое произошло при выполнении операции
	 */
	Image result() const;

	virtual ~Task() = default;

protected:
	/**
	 * Выполняет нужную операцию и возвращает результат
	 * @return Полученное изображение
	 * @throw Исключение в случае ошибки
	 */
	virtual Image on_execute() = 0;

private:
	//! обработчик уведомления о завершении задачи
	const CompleteHandler m_handler;

	//! возникшее в результате выполнения команды исключение
	std::exception_ptr m_exception;

	//! результат выполнения команды
	Image m_result;
};