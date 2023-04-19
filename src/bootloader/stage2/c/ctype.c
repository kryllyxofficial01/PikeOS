#include "std/int.h"
#include "std/ctype.h"

char upper(char character) {
    return islower(character) ? character - 'a' + 'A' : character;
}

bool islower(char character) {
    return character >= 'a' && character <= 'z';
}
