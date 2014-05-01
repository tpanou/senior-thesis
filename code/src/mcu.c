/**
@file
*/

#include "mcu.h"
#include "defs.h"

#include <avr/io.h>
#include <avr/interrupt.h>

/**
@brief Initializes MCU and resets all hardware.
*/
int main() {

    /** The tasks are outlined below: */

    /* Disable all interrupts during this procedure. */
    cli();

    /** - Setup CPU clock. */
    init_clock();

    /** - Setup USART prescaler and enable receiver and transmitter. */
    init_usart();

    sei();

    while(1)
        ;

    return 0;
}

void init_clock() {
    /* Clock Prescaler Change Enable bit (CLKPCE) of CLKPR must first be set
    while all other bits are cleared. */
    CLKPR       = _BV(CLKPCE);

    /* Then, within four clock cycles, the appropriate prescaler bits of the
    same register are set while the CLKPCE bit is cleared. For a clock
    frequency of 4MHz, only CLKPS1 needs to be set. Atmel pp.34--37. */
    CLKPR       = _BV(CLKPS1);
}

void init_usart() {
    /* Set UBRRn value. */
    UBRR0H      = (unsigned char)(UBRR_VALUE>>8);
    UBRR0L      = (unsigned char)(UBRR_VALUE);

    /* Set character size to 8 bits. */
    UCSR0C      = _BV(UCSZ01) | _BV(UCSZ00);

    /* Enable Rx-complete interrupts, Receiver and Transmitter. */
    UCSR0B      = _BV(RXCIE0) | _BV(RXEN0) | _BV(TXEN0);
}
