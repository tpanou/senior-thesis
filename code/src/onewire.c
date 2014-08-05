#include "onewire.h"

#include <avr/io.h>
#include <util/delay.h>

uint8_t w1_reset() {
    uint8_t presence;

    /* Note: it is safe to write 0 on an input pin. In contrast, 1 would enable
     * the internal pull-up resistor. */

    /* The bus master transmits a reset pulse (a low signal for 480--960μs).
    * *DS18B20 p.14* */
    W1_DQ_LOW();
    _delay_us(500);

    /* The bus master then goes into Rx mode. *DS18B20 p.14* */
    W1_DQ_RELEASE();

    /* The DS18B20 waits 15--60μs and then transmits the presence pulse (a low
     * signal for 60--240μs). *DS18B20 p.14*. Sample the line after 70μs,
    * during which the presence should have been set. */
    _delay_us(70);

    presence = W1_READ();

    /* Wait for a minimum of 480μs in Master Rx mode. *DS18B20 p.18*. 70μs have
    * already been spent. Explicitly leaving a total of 500μs. */
    _delay_us(430);

    return presence;
}

void w1_write(uint8_t byte) {
    uint8_t i           =  8;

    /* Data are transmitted LSB first (*DS18B20 p.5*). 8 iterations are
    * performed, at the end of each, @p byte is shifted to the right by one
    * bit. */
    while(i--) {

        W1_DQ_LOW();

        /* DQ must remain low for a minimum of 1 μs (*DS18B20 p.19). Then, if
        * writing @c 1, it should go high. Otherwise, it should remain low for
        * the remainder of the timeslot. */
        _delay_us(2);

        /* If writing @c 0, do not change the data-direction on the DQ pin.
        * Otherwise, do. In either case, wait for the remainder of the slot. */
        W1_DQ_DDR      &= ~((byte & 0x1)*_BV(W1_DQ));

        /* Slave sampling occurs within 30 to 60μs after DQ line goes low
        * (*DS18B20 p.19*). */
        _delay_us(55);

        /* Also, allow a minimum 1μs recovery between write slots (*DS18B20
        * p.19*). Explicitly leaving a total of 61μs. */
        W1_DQ_RELEASE();

        _delay_us(2);
        byte          >>= 1;
    }
}

uint16_t w1_read(uint8_t bits) {
    uint16_t data       =  0;       /* The output. */
    uint16_t mask       =  0x01;    /* Shifted left-wise, applied on @c data. */

    while(bits--) {
        /* Generate a Read time slot */

        /* A read slot is initiated with a low pulse of 1μs minimum
        * (*DS18B20 p19*) */
        W1_DQ_LOW();
        _delay_us(2);

        /* Release DQ line so the slave may gain control of it. */
        W1_DQ_RELEASE();

        /* Output data is valid for 15μs after the falling edge of the read time
        *slot (*DS18B20 p.19*). Read near the end of the 15μs window. */
        _delay_us(10);

        /* Append 1 to the buffer, if necessary. */
        if(W1_READ()) {
            data       |=  mask;
        }

        /* Allow for a slot of at least 60μs +1μs for recovery time (*DS18B20
        * p.19*). Explicitly leaving a total of 61μs. */
        _delay_us(49);

        mask <<= 1;
    }

    return data;
}
