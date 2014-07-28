#include "util.h"
#include <avr/pgmspace.h>
#include <stdarg.h>
#include <string.h>

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
