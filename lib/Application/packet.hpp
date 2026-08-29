#pragma once
#include <stdint.h>

constexpr uint16_t HTTP_BUFFER_SIZE = 2048;
struct HttpPacket {
    char response[HTTP_BUFFER_SIZE];
};