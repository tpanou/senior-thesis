/**
* @file
*/

#include "defs.h"
#include "w5100/w5100.h"

#include <avr/io.h>
#include <inttypes.h>

static uint8_t net_buffer[NET_BUF_SIZE];


void socket0_handler(uint8_t status) {

    printf("\nSOCKET STATUS: %x\n", status);

    /* If a connection has been established, just remove the flag. */
    if(status & Sn_IR_CON) {
        puts("Socket 0: CON\n");
    }

    if(status & Sn_IR_RECV) {
        uint16_t rx_size = getSn_RX_RSR(0);
        uint16_t fragment;

        uint16_t rx_RD;
        printf("Socket 0: RECV. TOTAL SIZE: %d:\n", rx_size);
        do {

            /* Read chunks from the incoming data, each up to a maximum of @c
            * NET_BUF_SIZE bytes. Later, each chunk will be forwarded to the
            * appropriate function. */
            fragment = rx_size < NET_BUF_SIZE ? rx_size : NET_BUF_SIZE- 1;
            recv(0, net_buffer, fragment);
            net_buffer[fragment] = '\0';

            printf("%s", net_buffer);

            rx_size    -= fragment;

        } while(rx_size > 0);

            rx_RD  = IINCHIP_READ(Sn_RX_RD0(0)) << 8;
            rx_RD |= IINCHIP_READ(Sn_RX_RD0(0) + 1) & 0x00FF;
        printf("\n>> RD: %x\n", rx_RD);
    }

    /* Occurs when a connection termination is requested OR completed. */
    if(status & Sn_IR_DISCON) {
        puts("Socket 0: DISCON\n");

    }
    if(status & Sn_IR_TIMEOUT) {
        puts("Socket 0: TIMEOUT\n");
    }

    puts("=========================");
}

