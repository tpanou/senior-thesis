
#include "w5100.h"
#include "defs.h"

#include <util/delay.h>
#include <avr/io.h>

/**
* @brief The absolute Tx address for each socket.
*
* Initialised by net_socket_init().
*/
static uint16_t tx_base[4];

/**
* @brief The address Tx mask for each socket.
*
* This value helps identify the relative offset within each buffer. Initialised
* by net_socket_init().
*/
static uint16_t tx_mask[4];

/**
* @brief The absolute Rx address for each socket.
*
* Initialised by net_socket_init().
*/
static uint16_t rx_base[4];

/**
* @brief The address Rx mask for each socket.
*
* This value helps identify the relative offset within each buffer. Initialised
* by net_socket_init().
*/
static uint16_t rx_mask[4];

/**
* @brief The amount of data stored in W5100 output buffer for each socket.
*
* Used by net_send() to facilitate sending data to W5100 without necessarily
* flushing them.
*/
static uint16_t socket_contents[4];

void net_socket_init(uint8_t tx, uint8_t rx) {
    uint16_t tx_sum =  0;       /* Sum of previously allocated Tx buffers. */
    uint16_t rx_sum =  0;       /* Sum of previously allocated Rx buffers. */
    uint16_t size;              /* Size of current buffer (either Tx or Rx). */
    uint8_t  i;

    net_write(NET_TMSR, &tx, 1);
    net_write(NET_RMSR, &rx, 1);

    /* Calculate the absolute start address of each Socket (sub)buffer as well
    * as the mask (Tx and Rx). */
    for(i = 0 ; i < 4 ; ++i) {

        /* The start address of this Socket's (sub)buffers depends on the size
        * of all previously allocated (sub)buffers (Tx and Rx). */

        size        =  1024 << (tx & 0x03);
        tx_base[i]  =  NET_TX_BASE + tx_sum;
        tx_mask[i]  =  size - 1;
        tx_sum     +=  size;

        size        =  1024 << (rx & 0x03);
        rx_base[i]  =  NET_RX_BASE + rx_sum;
        rx_mask[i]  =  size - 1;
        rx_sum     +=  size;

        /* Prepare the size bits for the next Socket. */
        tx  >>= 2;
        rx  >>= 2;
    }
}

void net_socket_open(uint8_t s, uint8_t mode, uint16_t port) {

    /* Close Socket @p s. */
    net_write8(NET_Sn_CR(s), NET_Sn_CR_CLOSE);
    net_write8(NET_Sn_MR(s), mode);
    net_write16(NET_Sn_PORT(s), port);
    net_write8(NET_Sn_CR(s), NET_Sn_CR_OPEN);

    /* Wait for the Socket to be opened. #NET_Sn_CR() clears automatically once
    * the command is executed. *W5100 p.27* */
    do {
    } while(net_read8(NET_Sn_CR(s)));
}

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
