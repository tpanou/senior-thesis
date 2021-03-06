#include "util.h"
#include "rtc.h"
#include "flash.h"

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

uint8_t temp_to_str(uint8_t* buf, uint8_t len, uint8_t t) {
    uint8_t digits;
    uint8_t i;

    digits  =  uint_to_str(buf + len - 1, t >> 1);
    for(i = 0 ; i < digits ; ++i) {
        buf[i]  =  buf[len - 1 - digits + i];
    }
    buf[i++]    =  '.';
    buf[i++]    =  t & 0x01 ? '5' : '0';
    buf[i]      =  0;

    return i;
}

uint8_t str_to_inet(uint8_t* ip, uint8_t* buf) {
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

void get_date(BCDDate* dt, uint8_t* day) {
    RTCMap rtc;
    rtc_get(&rtc);
    rtc_to_date(dt, &rtc);
    *day    =  rtc.day;
}

void set_date(BCDDate* dt, uint8_t day) {
    RTCMap rtc;
    date_to_rtc(&rtc, dt);
    rtc.day     =  day;
    rtc_set(&rtc);
}

int8_t str_to_date(BCDDate* dt, uint8_t* buf) {
    uint8_t num     =  0;
    uint8_t error   =  0;

    /* @p buf should contain a string of at least a full-date without fractions
    * of a second. */
    if(strlen(buf) < 19) return 1;

    error  +=  buf[0] != '2';
    error  +=  buf[1] != '0';

    error  += !isdigit(buf[2]);
    error  += !isdigit(buf[3]);
    dt->year = TO_BCD8(buf[2] - '0', buf[3] - '0');

    error  += buf[4] != '-';
    error  += !isdigit(buf[5]);
    error  += !isdigit(buf[6]);
    dt->mon = TO_BCD8(buf[5] - '0', buf[6] - '0');
    error  += dt->mon > 0x12;

    error  += buf[7] != '-';
    error  += !isdigit(buf[8]);
    error  += !isdigit(buf[9]);
    dt->date = TO_BCD8(buf[8] - '0', buf[9] - '0');
    error  += dt->date > 0x31;

    error  += buf[10] != 'T';
    error  += !isdigit(buf[11]);
    error  += !isdigit(buf[12]);
    dt->hour = TO_BCD8(buf[11] - '0', buf[12] - '0');
    error  += dt->hour > 0x23;

    error  += buf[13] != ':';
    error  += !isdigit(buf[14]);
    error  += !isdigit(buf[15]);
    dt->min = TO_BCD8(buf[14] - '0', buf[15] - '0');
    error  += dt->min > 0x59;

    error  += buf[16] != ':';
    error  += !isdigit(buf[17]);
    error  += !isdigit(buf[18]);
    dt->sec = TO_BCD8(buf[17] - '0', buf[18] - '0');
    error  += dt->sec > 0x59;

    return error > 0;
}

void date_to_str(uint8_t* buf, BCDDate* dt) {
    /* This is equivalent to the following: @verbatim
sprintf(buf, "20%02x-%02x-%02xT%02x:%02x:%02x.000Z", dt->year,
                                                     dt->mon,
                                                     dt->date,
                                                     dt->hour,
                                                     dt->min,
                                                     dt->sec);@endverbatim
    * but is faster, requires less memory and does not force a dependency on
    * vprintf(). */
    buf[0]  =  '2';
    buf[1]  =  '0';
    buf[2]  =  '0' + ((dt->year & 0xF0) >> 4);
    buf[3]  =  '0' +  (dt->year & 0x0F);
    buf[4]  =  '-';
    buf[5]  =  '0' + ((dt->mon & 0xF0)  >> 4);
    buf[6]  =  '0' +  (dt->mon & 0x0F);
    buf[7]  =  '-';
    buf[8]  =  '0' + ((dt->date & 0xF0) >> 4);
    buf[9]  =  '0' +  (dt->date & 0x0F);
    buf[10] =  'T';
    buf[11] =  '0' + ((dt->hour & 0xF0) >> 4);
    buf[12] =  '0' +  (dt->hour & 0x0F);
    buf[13] =  ':';
    buf[14] =  '0' + ((dt->min & 0xF0)  >> 4);
    buf[15] =  '0' +  (dt->min & 0x0F);
    buf[16] =  ':';
    buf[17] =  '0' + ((dt->sec & 0xF0)  >> 4);
    buf[18] =  '0' +  (dt->sec & 0x0F);
    buf[19] =  '.';
    buf[20] =  '0';
    buf[21] =  '0';
    buf[22] =  '0';
    buf[23] =  'Z';
    buf[24] =  0;
}

uint16_t pgm_read_str_array(uint8_t** indices, uint8_t* buf, ...) {
    va_list ap;         /* Pointer to each optional argument. */
    PGM_P str;          /* Address of a string in Flash. */
    uint8_t i = 0;      /* Index of @p indices to write to. */
    uint8_t* init = buf;

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
    return buf - init;
}

static inline void rtc_to_date(BCDDate* dt, RTCMap* rtc) {
    dt->year    =  rtc->year;
    dt->mon     =  rtc->mon;
    dt->date    =  rtc->date;
    dt->hour    =  rtc->hour;
    dt->min     =  rtc->min;
    dt->sec     =  rtc->sec;
}

static inline void date_to_rtc(RTCMap* rtc, BCDDate* dt) {
    rtc->year   =  dt->year;
    rtc->mon    =  dt->mon;
    rtc->date   =  dt->date;
    rtc->hour   =  dt->hour;
    rtc->min    =  dt->min;
    rtc->sec    =  dt->sec;
}

void fls_to_wiz(uint8_t s, uint16_t page, uint16_t len) {
    uint16_t size   =  256;
    uint8_t  buf[256];

    while(len) {
        if(len < size) size =  len;

        /* Read @c size bytes from the Flash. */
        fls_exchange(FLS_READ, page, buf, size);

        /* Send them to the W5100 HTTP server output buffer. */
        net_send(s, buf, size, 0);

        /* Prepare for the next iteration. */
        ++page;
        len    -=  size;
    }
}
