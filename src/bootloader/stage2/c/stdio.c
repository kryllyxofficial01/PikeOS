#include "std/io.h"
#include "std/int.h"
#include "x86.h"

const char hexChars[] = "0123456789abcdef";

int* printn(int* argp, int length, bool isSigned, int base);

// Print a character
void putc(char character) {
    x86_write_char(character, 0);
}

// Print a string
void puts(const char* string) {
    while (*string) {
        putc(*string);
        string++;
    }
}

// Print a formatted string
void printf(const char* string, ...) {
    int state = PRINTF_NORMAL_STATE;
    int length = PRINTF_DEFAULT_LENGTH;

    int base = 10;
    bool isSigned = false;

    int* argp = (int*) &string; // Points to the current argument
    argp++;

    while (*string) {
        switch (state) {
            case PRINTF_NORMAL_STATE:
                switch (*string) {
                    case '%':
                        state = PRINTF_LENGTH_STATE;
                        break;

                    default:
                        putc(*string);
                        break;
                }

                break;

            case PRINTF_LENGTH_STATE:
                switch (*string) {
                    case 'h':
                        length = PRINTF_SHORT_LENGTH;
                        state = PRINTF_SHORT_LENGTH_STATE;
                        break;

                    case 'l':
                        length = PRINTF_SHORT_LENGTH;
                        state = PRINTF_LONG_LENGTH_STATE;
                        break;

                    default:
                        goto PRINTF_SPEC_STATE_;
                }

                break;

            case PRINTF_SHORT_LENGTH_STATE:
                if (*string == 'h') {
                    length = PRINTF_SHORT_SHORT_LENGTH;
                    state = PRINTF_SPEC_STATE;
                }
                else {
                    goto PRINTF_SPEC_STATE_;
                }

                break;

            case PRINTF_LONG_LENGTH_STATE:
                if (*string == 'l') {
                    length = PRINTF_LONG_LONG_LENGTH;
                    state = PRINTF_SPEC_STATE;
                }
                else {
                    goto PRINTF_SPEC_STATE_;
                }

                break;

            case PRINTF_SPEC_STATE:
                PRINTF_SPEC_STATE_:
                    switch (*string) {
                        case 'c':
                            putc((char) *argp);
                            argp++;
                            break;

                        case 's':
                            puts(*(char**) argp);
                            argp++;
                            break;

                        case 'd':
                        case 'i':
                            base = 10;
                            isSigned = true;
                            argp = printn(argp, length, isSigned, base);
                            break;

                        case 'u':
                            base = 10;
                            isSigned = false;
                            argp = printn(argp, length, isSigned, base);
                            break;

                        case 'X':
                        case 'x':
                        case 'p':
                            base = 16;
                            isSigned = false;
                            argp = printn(argp, length, isSigned, base);
                            break;

                        case 'o':
                            base = 8;
                            isSigned = false;
                            argp = printn(argp, length, isSigned, base);
                            break;

                        case '%':
                            putc('%');
                            break;

                        default:
                            break;
                    }

                    state = PRINTF_NORMAL_STATE;
                    length = PRINTF_DEFAULT_LENGTH;
                    base = 10;
                    isSigned = false;

                    break;
        }

        string++;
    }
}

// Print a number
int* printn(int* argp, int length, bool isSigned, int base) {
    char buffer[32];
    unsigned long long number;
    int _signed = 1;
    int i = 0;

    switch (length) {
        case PRINTF_SHORT_SHORT_LENGTH:
        case PRINTF_SHORT_LENGTH:
        case PRINTF_DEFAULT_LENGTH:
            if (isSigned) {
                if (*argp < 0) {
                    number = -(*argp);
                    _signed = -1;
                }
                number = (unsigned long long) *argp;
            }
            else {
                number = *(unsigned int*) argp;
            }

            argp++;

            break;

        case PRINTF_LONG_LENGTH:
            if (isSigned) {
                if (*(long int*) *argp < 0) {
                    number = -(*argp);
                    _signed = -1;
                }
                number = (unsigned long long) (*(long int*) argp); // Type casting go brr
            }
            else {
                number = *(unsigned long int*) argp;
            }

            argp += 2;

            break;

        case PRINTF_LONG_LONG_LENGTH:
            if (isSigned) {
                if (*(long long int*) *argp < 0) {
                    number = -(*argp);
                    _signed = -1;
                }
                number = (unsigned long long) (*(long long int*) argp); // Type casting go brr
            }
            else {
                number = *(unsigned long long*) argp;
            }

            argp += 4;

            break;
    }

    do {
        uint32_t remainder;
        x86_div(number, base, &number, &remainder);
        buffer[i++] = hexChars[remainder];
    } while (number > 0);

    if (isSigned && _signed < 0) {
        buffer[i++] = '-';
    }

    while (--i >= 0) {
        putc(buffer[i]);
    }

    return argp;
}
