#pragma once

#include <cstdint>
#include <cstddef>

#pragma pack(1)

namespace proto
{

//! сигнатура, идентифицирующая протокол
static constexpr uint16_t Signature = 0x3fe0;

/**
 * @enum PayloadType тип содержимого пакета
 */
enum class PayloadType : uint8_t
{
	//! запрос печати текста на изображение
	kPrintTextRequest = 0,

	//! ответ на запрос с результатом
	kResponse
};

/**
 * @struct Header заголовок любого сообщения протокола
 */
struct Header
{
	//! сигнатура протокола
	uint16_t sign{Signature};

	//! тип пакета
	PayloadType type{PayloadType::kResponse};

	//! размер области данных
	uint32_t size{0};
};

/**
 * @struct PrintTextPayload структура запроса на печать текста на
 * изображении
 */
struct PrintTextPayload
{
	//! размер строки текста
	uint16_t text_size;

	//! размер изображения
	uint32_t image_size;
};

/**
 * @enum Status код ответа
 */
enum class Status : uint8_t
{
	//! все ок, изображение обработано
	kOk,

	//! сервер занят обработкой других задач
	kBusy,

	//! превышены пределы на размер изображения, строки и пр.
	kLimit,

	//! ошибка при обработке задачи
	kError
};

/**
 * @struct ResponsePayload формат сообщения о результате операции
 */
struct ResponsePayload
{
	//! результат выполнения запроса
	Status status;

	//! размер изображения
	uint32_t image_size;
};

} // namespace proto

#pragma pack()