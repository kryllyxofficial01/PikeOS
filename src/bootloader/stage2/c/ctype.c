#include "std/ctype.h"

bool islower(char character) {
    return character >= 'a' && character <= 'z';
}

char upper(char character) {
    return islower(character) ? (character - 'a' + 'A') : character;
}
