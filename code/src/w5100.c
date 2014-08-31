
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

uint16_t net_send(uint8_t s, uint8_t* buf, uint16_t len, uint8_t flush) {
    uint16_t free_size  =  net_read16(NET_Sn_TX_FSR(s));
    uint16_t s_content  =  socket_contents[s];

    /* Is there enough available space? */
    if(free_size < s_content + len) return free_size - s_content - len;

    /* Send data from local buffer to W5100 buffer. */

    uint16_t tx_WR      =  net_read16(NET_Sn_TX_WR(s));

    /* Offset from the (sub)buffer base. */
    uint16_t tx_offset  =  (tx_WR + s_content) & tx_mask[s];

    /* Physical address to start writing to. */
    uint16_t start_addr =  tx_base[s] + tx_offset;
    uint16_t sock_size  =  tx_mask[s] + 1;

    /* If the requested bytes (len) exceed the buffer limit, then overflow will
    * occur. Write data from current offset up to limit (upper-bound), and
    * then, the remainder of bytes starting from the (sub)buffer base. */
    if((tx_offset + len) > sock_size ) {
        /* Bytes until upper-bound. */
        uint16_t bound  = sock_size - tx_offset;

        /* Write bytes up to upper-bound. */
        net_write(start_addr, buf, bound);

        /* Write the rest of the bytes starting off from the base. */
        net_write(tx_base[s], buf + bound, len - bound);

    } else {
        net_write(start_addr, buf, len);
    }

    /* Update WR pointer for future operations. */

    s_content  +=  len;
    if(flush) {
        uint8_t status;
        tx_WR  +=  s_content;

        net_write16(NET_Sn_TX_WR(s), tx_WR);
        net_write8(NET_Sn_CR(s), NET_Sn_CR_SEND);

        do {
            status = net_read8(NET_Sn_IR(s));
        } while((status & NET_Sn_IR_SEND_OK) != NET_Sn_IR_SEND_OK);

        s_content = 0;
    }
    socket_contents[s]  =  s_content;
    return sock_size    -  s_content;
}

uint16_t net_recv(uint8_t s, uint8_t* buf, uint16_t len) {
    uint16_t rx_size    =  net_read16(NET_Sn_RX_RSR(s));
    uint16_t rx_RR      =  net_read16(NET_Sn_RX_RR(s));

    /* Read at most @p len bytes. */
    if(len < rx_size) rx_size = len;

    /* Offset from (sub)buffer base. */
    uint16_t rx_offset  =  rx_RR & rx_mask[s];

    /* Physical address to start reading from. */
    uint16_t start_addr =  rx_base[s] + rx_offset;
    uint16_t sock_size  =  rx_mask[s] + 1;

    /* If incoming (data size + current read offset) exceeds buffer limit, then
     * overflow has occurred. Read data from the current offset up to limit, and
     * then, the remainder of bytes from (sub)buffer base. */
    if(rx_offset + rx_size > sock_size) {
        /* Bytes until upper-bound. */
        uint16_t bound  = sock_size - rx_offset;

        /* Read bytes up to upper-bound. */
        net_read(start_addr, buf, bound);

        /* Read the rest of the bytes, starting off from the base. */
        net_read(rx_base[s], buf + bound, rx_size - bound);
    } else {
        net_read(start_addr, buf, rx_size);
    }

    /* Update RR pointer for future reads. */
    net_write16(NET_Sn_RX_RR(s), rx_RR + rx_size);
    net_write8(NET_Sn_CR(s), NET_Sn_CR_RECV);

    return rx_size - len;
}
