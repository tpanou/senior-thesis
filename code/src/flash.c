
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

void fls_command(uint8_t c, uint8_t* data) {
    fls_select();
    SPCR       |=  _BV(SPE);

    SPDR        =  c;
    loop_until_bit_is_set(SPSR, SPIF);

    if(data) {
        SPDR    = *data;
        loop_until_bit_is_set(SPSR, SPIF);
        *data   =  SPDR;
    }

    fls_deselect();
}

void fls_exchange(uint8_t c, uint16_t page, uint8_t* buf, uint8_t len) {
    uint8_t addr[3];
    uint8_t i;

    /* Calculate the starting address of @p page. */
    addr[0]     =  page >> 8;
    addr[1]     =  page & 0x0F;
    addr[2]     =  0;

    /* Send the command. */
    fls_select();
    SPCR       |=  _BV(SPE);
    SPDR        =  c;
    loop_until_bit_is_set(SPSR, SPIF);

    /* Send the address. This part could be merged with the next (sending
    * data).*/
    for(i = 0 ; i < 3; ++i) {
        SPDR    =  addr[i];
        loop_until_bit_is_set(SPSR, SPIF);
    }

    /* Send data, if any. */
    for(i = 0 ; i < len; ++i) {
        SPDR    =  buf[i];
        loop_until_bit_is_set(SPSR, SPIF);

        buf[i]  =  SPDR;
    }
    fls_deselect();
}
