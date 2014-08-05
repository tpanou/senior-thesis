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
