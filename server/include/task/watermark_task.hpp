#pragma once

#include <string>

#include "task.hpp"

/**
 * @class WatermarkTask задача наложения текста на изображение
 */
class WatermarkTask : public Task
{
public:
	/**
	 * Ctor
	 * @param handler Обработчик завершения задачи
	 * @param source_image Изображение
	 * @param text Текст для наложения
	 */
	WatermarkTask(Task::CompleteHandler&& handler, Image&& source_image,
				  std::string&& text);

protected:
	Image on_execute() override;

private:
	const Image       m_source;
	const std::string m_text;
};