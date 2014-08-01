#include "util.h"
#include <avr/pgmspace.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

uint8_t uint_to_str(uint8_t* buf, uint16_t number) {
    uint8_t unit;
    uint8_t i       =  0;

    *buf            =  '\0';
    while(number > 0) {
        ++i;
        *(buf - i)  =  number % 10 + '0';
        number     /=  10;
    }
    if(i == 0) {
        *(buf - 1)  = '0';
        i           =  1;
    }
    return i;
}

int8_t str_to_inet(uint8_t* ip, uint8_t* buf) {
    uint8_t  i;
    uint8_t* j = buf;
    char* last = NULL;

    for(i = 0 ; i < 4 && !last ; ++i) {
        ip[i] = strtol(j, &last, 10);

        if(*last == '.') {
            j = last + 1;
            last = NULL;
        }
    }
    if(i == 4) {
        return 0;
    }
    return 1;
}

uint8_t inet_to_str(uint8_t* buf, uint8_t* ip) {
    uint8_t byte;   /* A single byte from @p ip. */
    uint8_t i;      /* For each byte in @p ip. */
    uint8_t pos;    /* Position in @p buf to write to next. */
    uint8_t j;      /* Digit of @c byte to write next. */

    for(i = 0, pos = 0 ; i < 4 ; ++i) {
        byte = ip[i];
        j = 0;
        if(byte >= 10 && byte < 100) ++pos;
        else if(byte >= 100) pos += 2;

        /* Ensure this runs at least once so that a single zero may not be
        * omitted. */
        do {
            buf[pos - j] = byte % 10 + '0';
            ++j;
            byte /= 10;
        } while(byte);

        /* Increment for next '.' or terminating null-byte. */
        ++pos;
        /* If more bytes are to follow, place a '.' and further increase @c pos
        * to point at the position to write the next digit to. */
        if(i != 3) buf[pos++] = '.';
    }
    buf[pos] = '\0';
    return pos;
}

void pgm_read_str_array(uint8_t** indices, uint8_t* buf, ...) {
    va_list ap;         /* Pointer to each optional argument. */
    PGM_P str;          /* Address of a string in Flash. */
    uint8_t i = 0;      /* Index of @p indices to write to. */

    va_start(ap, buf);
    str = va_arg(ap, PGM_P);

    /* Repeat for all supplied progmem strings. */
    while(str != NULL) {
        indices[i]  =  buf;             /* Store start address of the string. */

        strcpy_P(buf, str);             /* Copy string into temporary buffer. */
        buf        +=  strlen(buf) + 1; /* Prepare the next buffer address. +1
        * is for null-byte. */

        ++i;
        str = va_arg(ap, PGM_P);
    }
}
