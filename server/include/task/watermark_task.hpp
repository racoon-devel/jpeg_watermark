#pragma once

#include <string>

#include "task.hpp"

/**
 * @class WatermarkTask задача наложения водяной печати на изображения
 */
class WatermarkTask : public Task
{
public:
	WatermarkTask(Task::CompleteHandler&& handler, Image&& source_image,
				  std::string&& text);

protected:
	Image on_execute() override;

private:
	const Image       m_source;
	const std::string m_text;
};