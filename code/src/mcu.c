/**
@file
*/

#include "mcu.h"
#include "defs.h"
#include "net.h"
#include "web_server.h"
#include "flash.h"

#include "w5100/w5100.h"
#include "w5100/socket.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>   /* e.g. loop_until_bit_is_set() */
#include <avr/sleep.h>

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

    DDRD        |= _BV(NET_RST);
    /* Assert W5100 RST pin low for at least 2us when power is first applied.
    * *WIZnet p.9.* */
    _delay_us(5);
    PORTD       |= _BV(NET_RST);

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

    set_host_name_ip(inet); /* Set the HTTP server host name. */

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

    set_sleep_mode(_BV(SM1));

    sei();

    while(1) {
        if(!task_pending()) {
            sleep_enable();
            sleep_cpu();
            sleep_disable();
        }
    }

    return 0;
}

/**
* @brief Initialise the various modules.
*
* It should be noted that this function sets the motor operating range
* (motor_set_max()). As a result, upon completion, the motors will be resetting.
*/
static void init() {
    uint8_t settings[]  =  {FACTORY_IADDR,
                            FACTORY_GATEWAY,
                            FACTORY_SUBNET,
                            FACTORY_HADDR,
                            GRID_X_LEN, GRID_Y_LEN, GRID_Z_LEN};
    Position max;

    uint8_t rtc_sec;
    rtc_read(0, &rtc_sec, 1);

    /* If bit #RTC_CH is set, then the clock is not running. The implementation
    * never stops the RTC from running, so it may just be the battery supply was
    * removed to reset to factory (default) settings. */
    if(bit_is_set(rtc_sec, RTC_CH)) {

        /* Write factory settings into the RTC memory. */
        rtc_write(RTC_BASE, settings, 21);

    /* Load battery-backed (RTC) settings. */
    } else {

        rtc_read(RTC_BASE, settings, 21);
    }

    max.x = settings[SYS_MTR_MAX_X - RTC_BASE];
    max.y = settings[SYS_MTR_MAX_Y - RTC_BASE];
    max.z = settings[SYS_MTR_MAX_Z - RTC_BASE];

    /* Network module */
    /* Setup buffer size. #HTTP_SOCKET is configured to 8KB on Tx and Rx). */
    net_socket_init(NET_SIZEn(HTTP_SOCKET, NET_SIZE_8),
                    NET_SIZEn(HTTP_SOCKET, NET_SIZE_8));

    /* Pass server settings to the W5100 and the HTTP server module. */
    srvr_set_host_name_ip(  &settings[SYS_IADDR -   RTC_BASE]);
    net_write(NET_SIPR,     &settings[SYS_IADDR -   RTC_BASE], 4);
    net_write(NET_GAR,      &settings[SYS_GATEWAY - RTC_BASE], 4);
    net_write(NET_SUBR,     &settings[SYS_SUBNET -  RTC_BASE], 4);
    net_write(NET_SHAR,     &settings[SYS_HADDR -   RTC_BASE], 6);

    /* Mode register (MR) defaults look OK. The same applies for RTR (200ms
    * intervals) and RCR (8 retries). */

    /* Enable interrupts on HTTP_SOCKET. */
    net_write8(NET_IMR, NET_IR_Sn(HTTP_SOCKET));

    /* Setup #HTTP_SOCKET for HTTP (TCP on port 80). */
    net_socket_open(HTTP_SOCKET, NET_Sn_MR_TCP, 80);
    net_write8(NET_Sn_CR(HTTP_SOCKET), NET_Sn_CR_LISTEN);

    /* Other modules */
    task_init();
    srvr_init();
    rsrc_init();
    log_init();
    motor_init();

    /* Set operating range *after* the motors have been initialised. */
    motor_set_max(&max);
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
    uint8_t     buf[256];
    uint16_t    page;
    uint16_t    len;
    uint16_t    i;

    page    =  usart_getchar(stdin);
    page    =  ((uint16_t)usart_getchar(stdin) << 8) | page;

    len     =  usart_getchar(stdin);
    len     =  ((uint16_t)usart_getchar(stdin) << 8) | len;

    /* Load data into a local buffer. */
    for(i = 0 ; i < len ; ++i) {
        buf[i]  =  usart_getchar(stdin);
    }

    /* Wait until the Flash has completed all previous write operations. */
    fls_wait_WIP();

    /* Enable Latch and send data to the Flash. */
    fls_command(FLS_WREN, NULL);
    fls_exchange(FLS_WRITE, page, buf, len);

    printf(" [page:%d,len:%d]\n\n", page, len);
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
