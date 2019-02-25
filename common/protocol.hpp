#pragma once

#include <cstdint>
#include <cstddef>

#pragma pack()

#define PROTO_VALID_SIGN (0x3fe0) // сигнатура для проверка - для нашего ли сервера отправлен пакет

struct ProtoDataHeader 
{
    uint16_t sign; // сигнатура
    uint16_t text_size; // Размер текстовой строки
    size_t image_size; // Размер изображения
};

#pragma pack(pop)