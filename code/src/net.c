/**
* @file
*/

#include "defs.h"
#include "w5100/w5100.h"
#include "web_server.h"
#include "sbuffer.h"

#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>

void socket0_handler(uint8_t status) {

    /* Print interrupt flags for this invocation. */
    printf("\nSOCKET 0 Sn_IR: 0x%x (", status);
    if(status & Sn_IR_SEND_OK) printf("[SEND_OK]");
    if(status & Sn_IR_TIMEOUT) printf("[TIMEOUT]");
    if(status & Sn_IR_RECV) printf("[RECV]");
    if(status & Sn_IR_DISCON) printf("[DISCON]");
    if(status & Sn_IR_CON) printf("[CON]");
    printf(") Sn_SR: 0x%x\n", IINCHIP_READ(Sn_SR(0)));

    if(status & Sn_IR_RECV) {

        if(getSn_RX_RSR(0) > 0 ) {
            set_socket_buf(0);
            http_parse_request();
        }
    }

    /* Occurs when a connection termination is requested OR completed. */
    if(status & Sn_IR_DISCON) {
        uint8_t sn_SR   = IINCHIP_READ(Sn_SR(0));

        printf("[DISCON] Sn_SR: %x\n", sn_SR);

        /* Termination request is received from peer host. Close the connection
        * by sending a DISCON or CLOSE command. *WIZnet p.30* */
        if(sn_SR == SOCK_CLOSE_WAIT || sn_SR == SOCK_CLOSED) {
            /* Issue a DISCON command as a response to FIN+ACK. */
            disconnect(0);
            close(0);

            printf("> Socket closed. Sn_SR: %x\n", IINCHIP_READ(Sn_SR(0)));

            /* Re-open socket. */
            if(socket(0, Sn_MR_TCP, 80, 0)) {
                /* Set port to listen for requests. */
                listen(0);
                puts(" > HTTP socket has been re-opened.");
            }
        }

    }
    if(status & Sn_IR_TIMEOUT) {
        puts("[TIMEOUT] Timeout has occurred.");
    }

    /* Clear interrupt flags that have been dealt with. */
    IINCHIP_WRITE(Sn_IR(0), status);

    puts("=========================");
}

