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

void handle_http_socket(uint8_t s, uint8_t status) {

    /* Data available. */
    if(bit_is_set(status, NET_Sn_IR_RECV)) {

        if(net_read16(NET_Sn_RX_RSR(s)) > 0 ) {
            uint8_t c;      /* Discarded character. */
            int8_t  c_type;

            /* Stream data from this Socket to local (host) buffering. */
            set_socket_buf(s);

            /* Service HTTP request. */
            srvr_call();

            /* Discard the remainder of the request. */
            while((c_type = s_next(&c)) != -1) {
            }
        }
    }

    /* Occurs when a connection termination is requested OR completed. */
    if(bit_is_set(status, NET_Sn_IR_DISCON)) {
        uint8_t sn_SR   =  net_read8(NET_Sn_SR(s));

        /* Termination request is received from peer host. Close the connection
        * by sending a DISCON or CLOSE command. *WIZnet p.30* */
        if(sn_SR == NET_Sn_SR_CLOSEWAIT || sn_SR == NET_Sn_SR_CLOSED) {
            /* Issue a DISCON command as a response to FIN+ACK. */
            net_write8(NET_Sn_CR(s), NET_Sn_CR_DISCON);
            net_write8(NET_Sn_CR(s), NET_Sn_CR_CLOSE);

            /* Re-open socket. */
            net_socket_open(HTTP_SOCKET, NET_Sn_MR_TCP, 80);
            net_write8(NET_Sn_CR(s), NET_Sn_CR_LISTEN);
        }

    }
    if(bit_is_set(status, NET_Sn_IR_TIMEOUT)) {
        net_write8(NET_Sn_CR(s), NET_Sn_CR_DISCON);
    }
}

