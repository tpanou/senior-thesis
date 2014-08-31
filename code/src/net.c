#include "net.h"
#include "defs.h"
#include "w5100.h"
#include "http_server.h"

#include <avr/io.h>
#include <avr/interrupt.h>


/**
* @brief Propagates interrupts from the W5100 to the appropriate handlers.
*
* This ISR reads the #NET_IR to determine the socket source and calls the
* appropriate handler. Currently, only one socket operates, which is responsible
* for the HTTP server transactions (see handle_http_socket()). Upon invoking the
* handler, the ISR keeps a copy of the socket's interrupt register (#NET_Sn_IR)
* and passes its value as an argument. The handler is supposed to respond to the
* specified flags only. Once completed, the ISR will clear the interrupt flag
* bits that were specified to the handler. If more interrupt flag bits have been
* set in the meantime, the ISR will be ready to respond to them.
*/
ISR(INT1_vect) {
    uint8_t status = net_read8(NET_IR);
    uint8_t socket;

    /* Only HTTP_SOCKET interrupts are of interest; clear high nibble. */
    net_write8(NET_IR, 0xE0);

    if(bit_is_set(status, HTTP_SOCKET)) {
        socket  =  net_read8(NET_Sn_IR(HTTP_SOCKET));
        handle_http_socket(HTTP_SOCKET, socket);

        /* Clear Socket interrupt flags that have just been dealt with. */
        net_write8(NET_Sn_IR(HTTP_SOCKET), socket);
    }
}

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

            srvr_call();
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

