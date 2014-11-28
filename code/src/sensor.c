#include "sensor.h"
#include "onewire.h"

uint16_t sens_read_t() {
    uint16_t t;

    W1_ENABLE();

    /* All transactions on the 1-wire bus begin with an initialisation sequence
    * (*DS18B20 p.11*). Do not proceed if no devices were detected, and in this
    * case, the DS18B20) . */
    if(w1_reset()) return -1;

    /* Currently, we operate in a single drop bus system so skip ROM
    * operations. *DS18B20 p.11* */
    w1_write(W1_ROM_SKIP);
    w1_write(W1_CONVERT_T);

    /* Allow for a 12-bit-resolution conversion to complete. *DS18B20 p.9* */
    _delay_ms(750);

    if(w1_reset()) return -1;

    w1_write(W1_ROM_SKIP);
    w1_write(W1_READ_SCRATCHPAD);
    t       =  w1_read(12);

    /* Halt DS18B20 operation. */
    w1_reset();

    W1_DISABLE();
    return t;
}

