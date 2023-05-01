#include "std/memory.h"

void far* memcpy(void far* destination, const void far* source, uint16_t size) {
    uint8_t far* _destination = (uint8_t far*) destination;
    const uint8_t far* _source = (const uint8_t far*) source;

    for (uint16_t i = 0; i < size; i++) {
        _destination[i] = _source[i];
    }

    return destination;
}

void far* memset(void far* ptr, int value, uint16_t size) {
    uint8_t far* _ptr = (uint8_t far*) ptr;

    for (uint16_t i = 0; i < size; i++) {
        _ptr[i] = value;
    }

    return ptr;
}

int memcmp(const void far* ptrA, const void far* ptrB, uint16_t size) {
    const uint8_t far* _ptrA = (const uint8_t far*) ptrA;
    const uint8_t far* _ptrB = (const uint8_t far*) ptrB;

    for (uint16_t i = 0; i < size; i++) {
        if (_ptrA[i] != _ptrB[i]) {
            return 1;
        }
    }

    return 0;
}
