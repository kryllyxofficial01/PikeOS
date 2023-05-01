#include "std/string.h"

const char* strchr(const char* string, char character) {
    if (string == NULL) {
        return NULL;
    }

    while (*string) {
        if (*string == character) {
            return string;
        }

        ++string;
    }

    return NULL;
}

char* strcpy(char* destination, const char* source) {
    char* _destination = destination;

    if (destination == NULL) {
        return NULL;
    }

    if (source == NULL) {
        *destination = '\0';
        return destination;
    }

    while (*source) {
        *destination = *source;
        ++source;
        ++destination;
    }

    *destination = '\0';
    return _destination;
}

unsigned strlen(const char* string) {
    unsigned length = 0;

    while (*string) {
        ++length;
        ++string;
    }

    return length;
}
