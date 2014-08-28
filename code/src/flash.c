
#include "flash.h"
#include <util/delay.h>

void fls_select() {
    /* Disable SPI, if running. */
    SPCR        = 0;

    /* Use the specified clock settings in SPI Master mode (0,0). Do *not*
    * enable. */
    SPSR        =  FLS_SPSR &  _BV(SPI2X);
    SPCR        = (FLS_SPCR & (_BV(SPR1) | _BV(SPR0))) | _BV(MSTR);

    /* Set the appropriate signals to select the Flash memory (through the mux).
    * Wait for a minimum of 25ns (if V_{CC} is 4.5--5.5) before sending any CLK
    * pulses. (T_{CSS} - @c nCS setup time, *25LC1024 p.3*) */
    FLS_ENABLE();
    _delay_us(1);
}

void fls_deselect() {

    /* Wait before pulling @c nCS high (T_{CSH} 50ns, *25LC1024 p.3*). */
    _delay_us(1);
    FLS_DISABLE();

    /* Wait the appropriate amount of time for Flash to release MISO line after
    * deselecting it. (T_{DIS} - Output disable time, *25LC1024 p.4*). */
    _delay_us(1);
}
