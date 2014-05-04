/**
@file
*/

#include "mcu.h"
#include "defs.h"
#include "net.h"

#include "w5100/w5100.h"
#include "w5100/socket.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>   /* e.g. loop_until_bit_is_set() */

#include <util/delay.h>

/**
@brief Initializes MCU and resets all hardware.
*/
int main() {

    /** The tasks are outlined below: */

    /* Disable all interrupts during this procedure. */
    cli();

    /** - Setup CPU clock. */
    init_clock();

    /** - Setup USART prescaler and enable receiver and transmitter. */
    init_usart();

    /** - Setup I/O streams. */
    stdout      = &usart_output;
    stdin       = &usart_input;

    DDRD |= _BV(NET_RST); // NET_RST 
    /* Assert W5100 RST pin low for at least 2us when power is first applied.
    * *WIZnet p.9.* */
    _delay_us(5);
    PORTD |= _BV(NET_RST);

    /** - Enable external interrupts on INT0 (on low level, by default). *Atmel
    * p.72.* */
    EIMSK       |= _BV(INT0);

    /* Setup buffer size for each socket (for now, 2kBytes each, both on Rx and
    * Tx). */
    sysinit(0x55, 0x55);

    uint8 gateway[]     = {192, 168,   1, 1};
    uint8 subnet[]      = {255, 255, 255, 0};
    uint8 inet[]        = {192, 168,   1, 73};
    uint8 mac[]         = {0xBE, 0xEF, 0xBE, 0xEF, 0xBE, 0xEF};

    setGAR(gateway);
    setSIPR(inet);
    setSUBR(subnet);    /* This merely sets a global variable. */
    applySUBR(subnet);  /* This actually passes the address into W5100. */
    setSHAR(mac);

    /* Mode register (MR) defaults look OK. The same applies for RTR (200ms
    * intervals) and RCR (8 retries). */

    IINCHIP_WRITE(IMR, IR_SOCK(0)); /* Enable interrupts for socket 0. */

    socket0_handler_ptr = &socket0_handler;

    /* Setup socket `s' for HTTP (TCP on port 80). */
    if(socket(0, Sn_MR_TCP, 80, 0)) {
        /* Set port to listen for requests. */
        listen(0);

        puts("HTTP socket has been opened and is waiting for connections.");
    }

    sei();

    while(1)
        ;

    return 0;
}

int usart_putchar(char c, FILE* stream) {
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0        = c;
    return 0;
}

int usart_getchar(FILE* stream) {
    loop_until_bit_is_set(UCSR0A, RXC0);
    return UDR0;
}

ISR(USART_RX_vect) {

    char buf[100];
    if(gets(buf)) {
        /* For the time being, just echo back. */
        /* NL is not appended after the string so that the host does not
        flush before receiving the rest of the message. */
        fputs("You sent: ", stdout);
        puts(buf);

    } else {
        puts("Error.");
    }
}

void init_clock() {
    /* Clock Prescaler Change Enable bit (CLKPCE) of CLKPR must first be set
    while all other bits are cleared. */
    CLKPR       = _BV(CLKPCE);

    /* Then, within four clock cycles, the appropriate prescaler bits of the
    same register are set while the CLKPCE bit is cleared. For a clock
    frequency of 4MHz, only CLKPS1 needs to be set. Atmel pp.34--37. */
    CLKPR       = _BV(CLKPS1);
}

void init_usart() {
    /* Set UBRRn value. */
    UBRR0H      = (unsigned char)(UBRR_VALUE>>8);
    UBRR0L      = (unsigned char)(UBRR_VALUE);

    /* Set character size to 8 bits. */
    UCSR0C      = _BV(UCSZ01) | _BV(UCSZ00);

    /* Enable Rx-complete interrupts, Receiver and Transmitter. */
    UCSR0B      = _BV(RXCIE0) | _BV(RXEN0) | _BV(TXEN0);
}
