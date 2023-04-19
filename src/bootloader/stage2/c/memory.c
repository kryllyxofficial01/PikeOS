#include "std/int.h"
#include "std/memory.h"

void far* memcpy(const void far* source, void far* destination, uint16_t length) {
    uint8_t far* _destination = (uint8_t far*) destination;
    const uint8_t far* _source = (const uint8_t far*) source;

    for (uint16_t i = 0; i < length; i++) {
        _destination[i] = _source[i];
    }

    return destination;
}

void far* memset(void far* pointer, int value, uint16_t size) {
    uint8_t far* _pointer = (uint8_t far*) pointer;

    for (uint16_t i = 0; i < size; i++) {
        _pointer[i] = value;
    }

    return pointer;
}

bool memcmp(const void far* pointer1, const void far* pointer2, uint16_t length) {
    const uint8_t far* _pointer1 = (const uint8_t far*) pointer1;
    const uint8_t far* _pointer2 = (const uint8_t far*) pointer2;

    for (uint16_t i = 0; i < length; i++) {
        if (_pointer1[i] != _pointer2[i]) {
            return true;
        }
    }

    return false;
}
