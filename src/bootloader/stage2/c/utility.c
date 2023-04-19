#include "std/int.h"
#include "std/utility.h"

uint32_t align(uint32_t number, uint32_t alignment) {
    if (alignment == 0) {
        return number;
    }

    uint32_t remainder = number % alignment;
    return remainder > 0 ? (number + alignment - remainder) : number;
}
