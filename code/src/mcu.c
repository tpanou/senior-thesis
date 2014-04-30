/**
@file
*/

#include <defs.h>

#include <avr/io.h>
#include <avr/interrupt.h>

/**
@brief Initializes MCU and resets all hardware.
*/
int main() {

    /** The tasks are outlined below: */
    /** - Setup CPU clock. */
    /* Disable all interrupts during this procedure. */
    cli();
    /* Clock Prescaler Change Enable bit (CLKPCE) of CLKPR must first be set
    while all other bits are cleared. */
    CLKPCE      = _BV(CLKPCE);
    /* Then, within four clock cycles, the appropriate prescaler bits of the
    same register are set while the CLKPCE bit is cleared. For a clock
    frequency of 4MHz, only CLKPS1 needs to be set. Atmel pp.34--37. */
    CLKPCE      = _BV(CLKPS1);
    sei();
    return 0;
}
