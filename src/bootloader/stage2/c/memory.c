#include "std/memory.h"

int memcpy(void far* source, const void far* destination, uint16_t length) {
    uint8_t far* _destination = (uint8_t far*) destination;
    const uint8_t far* _source = (const uint8_t far*) source;

    for (uint16_t i = 0; i < length; i++) {
        _destination[i] = _source[i];
    }
}