#include "task/watermark_task.hpp"

#include "easylogging++.h"
#include <jpeglib.h>
#include <jerror.h>
#define cimg_display 0
#define cimg_plugin  "plugins/jpeg_buffer.h"
#define cimg_use_pthread
#include "CImg.h"

WatermarkTask::WatermarkTask(Task::CompleteHandler&& handler,
							 Image&& source_image, std::string&& text)
	: Task(std::move(handler)), m_source(std::move(source_image)),
	  m_text(std::move(text))
{
}

Image WatermarkTask::on_execute()
{
	cimg_library::CImg< uint8_t > img;
	Image                         result;

	LOG(DEBUG) << "Drawing text '" << m_text << "' on image...";

	img.load_jpeg_buffer(&m_source[0], m_source.size());

	const unsigned char green[] = {0, 255, 0};
	img.draw_text(0, 0, m_text.c_str(), green, 0, 1, 57);

	result.resize(img.size());
	uint size = result.size();
	img.save_jpeg_buffer(&result[0], size, 90);

	result.resize(size, 0);
	return result;
}
