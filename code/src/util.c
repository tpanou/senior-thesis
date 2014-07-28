#include "util.h"

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
