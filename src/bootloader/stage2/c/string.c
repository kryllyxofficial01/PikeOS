#include "std/string.h"
#include "std/memory.h"

const char* find_char(const char* string, char character) {
    if (string == NULL) {
        return NULL;
    }

    while (*string) {
        if (*string == character) {
            return string;
        }

        string++;
    }

    return NULL;
}

char* copy(const char* source, char* destination) {
    char* _destination = destination;

    if (source == NULL) {
        *destination = '\0';
        return destination;
    }

    if (destination == NULL) {
        return NULL;
    }

    while (*source) {
        *destination = *source;
        source++;
        destination++;
    }

    *destination = '\0';
    return _destination;
}

int len(const char* string) {
    unsigned length = 0;

    while (*string) {
        length++;
        string++;
    }

    return length;
}