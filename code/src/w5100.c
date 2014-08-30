
#include "w5100.h"
#include "defs.h"

#include <util/delay.h>
#include <avr/io.h>

void net_select() {
    /* Disable SPI, if running. */
    SPCR        =  0;

    /* Use the specified clock settings in SPI Master mode (0,0). Do *not*
    * enable. */
    SPSR        =  NET_SPSR &  _BV(SPI2X);
    SPCR        = (NET_SPCR & (_BV(SPR1) | _BV(SPR0))) | _BV(MSTR);
}

void inline net_write8(uint16_t addr, uint8_t data) {
    net_exchange(0xF0, addr, &data, 1);
}

uint8_t net_read8(uint16_t addr) {
    uint8_t data;
    net_exchange(0x0F, addr, &data, 1);
    return data;
}

uint16_t net_read16(uint16_t addr) {
    uint16_t data;
    data    =  ((uint16_t)net_read8(addr)) << 8;
    data   |=  net_read8(addr + 1);
    return data;
}

void net_write16(uint16_t addr, uint16_t data) {
    net_write8(addr, data >> 8);
    net_write8(addr + 1, data);
}

void inline net_write(uint16_t addr, uint8_t* buf, uint8_t len) {
    net_exchange(0xF0, addr, buf, len);
}

void inline net_read(uint16_t addr, uint8_t* buf, uint8_t len) {
    net_exchange(0x0F, addr, buf, len);
}

void net_exchange(uint8_t c, uint16_t addr, uint8_t* buf, uint8_t len) {
    uint8_t update  = c == 0x0F;
    uint8_t byte;
    uint8_t i;

    net_select();
    SPCR       |=  _BV(SPE);

    for(i = 0 ; i < len ; ++i) {
        /* Set the appropriate signals to select the W5100. Wait for a minimum
        * of 21ns before sending any CLK pulses. *W5100 p.67* */
        NET_ENABLE();

        SPDR    = c;
        loop_until_bit_is_set(SPSR, SPIF);

        SPDR    = addr>>8;
        loop_until_bit_is_set(SPSR, SPIF);

        SPDR    = addr & 0x00FF;
        loop_until_bit_is_set(SPSR, SPIF);

        SPDR    = buf[i];
        loop_until_bit_is_set(SPSR, SPIF);

        if(update) {
            buf[i] = SPDR;
        }

        ++addr;

        NET_DISABLE();
    }

    /* Delay before releasing control. *W5100 p.67* */
    _delay_us(1);
    SPCR       &= ~_BV(SPE);
}
