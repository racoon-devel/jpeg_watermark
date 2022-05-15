#pragma once

#include <cstdint>
#include <cstddef>

#pragma pack(1)

#define PROTO_VALID_SIGN \
	(0x3fe0) // сигнатура для проверка - для нашего ли сервера отправлен пакет

struct ProtoDataHeader
{
	uint16_t sign;       // сигнатура
	uint16_t text_size;  // Размер текстовой строки
	size_t   image_size; // Размер изображения
};

enum StatusCode
{
	kStatusOK,    // Все ок, картинка обработана
	kStatusBusy,  // Сервер занят
	kStatusLimit, // Превышен лимит размера данных
	kStatusError  // Ошибка на стороне сервера
};

struct ProtoResponseHeader
{
	StatusCode code;
	size_t     image_size;
};

#pragma pack()