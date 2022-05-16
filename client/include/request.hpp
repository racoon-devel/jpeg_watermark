#pragma once

#include <string>

#include "image.hpp"
#include "protocol.hpp"

/**
 * @struct PrintTextRequest хелпер для создания запроса печати текста на
 * изображение
 */
struct PrintTextRequest
{
	//! текст
	std::string text;

	//! изображение
	Image image;

	//! тип сообщения протокола
	static constexpr proto::PayloadType payload_type()
	{
		return proto::PayloadType::kPrintTextRequest;
	}

	//! сериализовать в байт буфер
	auto serialize()
	{
		std::vector< uint8_t > result;
		result.resize(sizeof(proto::PrintTextPayload) + text.size()
					  + image.size());

		auto payload =
			reinterpret_cast< proto::PrintTextPayload* >(result.data());
		payload->text_size  = text.size();
		payload->image_size = image.size();

		auto pos = std::copy(text.begin(), text.end(),
							 result.begin() + sizeof(*payload));
		std::copy(image.begin(), image.end(), pos);

		return result;
	}
};