/**
@file
*/

#include "mcu.h"
#include "defs.h"

#include "net.h"
#include "http_server.h"
#include "resource.h"
#include "w5100.h"

#if defined (ENABLE_SERIAL_IO) && !defined (ENABLE_DEBUG)
#include "flash.h"
#endif

#include "task.h"
#include "motor.h"
#include "twi.h"
#include "rtc.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <util/delay.h>
#include <inttypes.h>

#ifdef ENABLE_SERIAL_IO
/**
* @brief Output stream over the USART.
*
* This stream is configured as the default output and error streams (stdout,
* stderr).
*/
FILE usart_output = FDEV_SETUP_STREAM(usart_putchar, NULL, _FDEV_SETUP_WRITE);

/**
* @brief Input stream over the USART.
*
* This stream is configured as the default input stream (stdin).
*/
FILE usart_input  = FDEV_SETUP_STREAM(NULL, usart_getchar, _FDEV_SETUP_READ);
#endif /* defined (ENABLE_SERIAL_IO) */

/**
* @brief Initialise MCU and resets all hardware.
*/
int main() {

    /** The tasks are outlined below: */

    /* Disable all interrupts during this procedure. */
    cli();

    /* Always disable the Watchdog Timer, even if it is not used (*Atmel p.52*).
    * If set, bit @c WRDF of @c MCUSR overrides bit @c WDE of @c WDTCR, so it
    * needs to be cleared first (*Atmel p.55*). */
    MCUSR      &= ~_BV(WDRF);
    /* To change @c WDE (and/or the prescaler bits), bit @c WDCE of WDTCSR must
    * be set first and the change must occur within four cycles, after which,
    * @c WDCE resets (*Atmel p.55*). What is not mentioned, though, is that bit
    * @c WDE must also be set. */
    WDTCSR      =  _BV(WDCE) | _BV(WDE);
    WDTCSR      =  _BV(WDCE);

    /** - Setup CPU clock. */
    init_clock();

#ifdef ENABLE_SERIAL_IO
    /** - Setup USART prescaler and enable receiver and transmitter. */
    init_usart();

    /** - Setup I/O streams. */
    stdout      = &usart_output;
    stdin       = &usart_input;
#endif

    _delay_ms(2000);

    /* When in Master SPI mode, if SS is input low, MSTR bit will be cleared. */
    DDRB       |= _BV(DDB2);

    /** - Enable external interrupts on INT1 (on low level, by default). *Atmel
    * p.72* */
    EIMSK      |= _BV(INT1);

    /* Pins connected to @c nCS, @c S0 and @c S1 of @c MUX are used as output,
    * while the pin connected to @c 2Z is used as input. Pins for the optical
    * encoder enable are also set. XZ-motor enable pins need not be set as
    * output as well because they are physically connected to @c S0 and @c S1 of
    * @c MUX. */
    MUX_nCS_DDR    |=  _BV(MUX_nCS);
    MUX_S0_DDR     |=  _BV(MUX_S0);
    MUX_S1_DDR     |=  _BV(MUX_S1);
    MUX_2Z_DDR     &= ~_BV(MUX_2Z);

    /* Have MUX disabled, by default. */
    MUX_DISABLE();

    /* SCLK, MOSI */
    DDRB   |= _BV(DDB5) | _BV(DDB3);
    DDRD   |= _BV(DDD7);

    /* Output to control nCS of Flash. */
    DDRD       |=  _BV(DDD1);
    PORTD      |=  _BV(PORTD1);

    /* Initialise the remaining modules. */
    init();

    set_sleep_mode(_BV(SM1));

    /* Set both Change Enable *and* System Reset Mode bits to enable setting the
    * timeout. Then, the WDT is set to Interrupt (only) mode. This way, at
    * #WDT_TIMEOUT intervals, the CPU will be woken from power-down mode. The
    * WDT ISR checks whether sampling should be initiated. Once execution
    * returns to the main loop, the CPU goes to power-down, again. The CPU maybe
    * woken at any time by other sources, as well, such as a limit switch and/or
    * an incoming HTTP request. Those requests could delay the CPU for as long
    * they require with no fear of the WDT timeout; since the System Reset Mode
    * is not activated, the WDT Interrupt will simply be queued, if it occurs
    * before any previously activated ISR returns. */
    WDTCSR  =  _BV(WDCE) | _BV(WDE);
    WDTCSR  =  _BV(WDCE) | _BV(WDIE) | (WDT_TIMEOUT & (_BV(WDP3)
                                                     | _BV(WDP2)
                                                     | _BV(WDP1)
                                                     | _BV(WDP0)));

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

void sys_get(uint8_t setting, void* value) {
    switch(setting) {
        case SYS_IADDR:
            net_read(NET_SIPR, value, 4);

        break;
        case SYS_GATEWAY:
            net_read(NET_GAR, value, 4);

        break;
        case SYS_SUBNET:
            net_read(NET_SUBR, value, 4);

        break;
        case SYS_HADDR:
            net_read(NET_SHAR, value, 6);

        break;
        case SYS_MTR_MAX:
            motor_get_max(value);

        break;
        case SYS_TASK:
            task_get(value);
        break;
    }
}

int8_t sys_set(uint8_t setting, void* value) {
    int8_t ret  =  -1;

    switch(setting) {
        case SYS_IADDR:
            net_write(NET_SIPR, value, 4);
            ret =  rtc_write(SYS_IADDR, value, 4);
            srvr_set_host_name_ip(value);

        break;
        case SYS_GATEWAY:
            net_write(NET_GAR, value, 4);
            ret =  rtc_write(SYS_GATEWAY, value, 4);

        break;
        case SYS_SUBNET:
            net_write(NET_SUBR, value, 4);
            ret =  rtc_write(SYS_SUBNET, value, 4);

        break;
        case SYS_HADDR:
            net_write(NET_SHAR, value, 6);
            ret =  rtc_write(SYS_HADDR, value, 6);

        break;
        case SYS_MTR_MAX:
            motor_set_max(value);
            ret =  rtc_write(SYS_MTR_MAX, value, 3);
        break;
        case SYS_TASK:
            task_set(value);
            ret =  rtc_write(SYS_TASK, value, 2);
        break;

    }

    return ret;
}

static void init() {
    uint8_t settings[]  =  {FACTORY_IADDR,
                            FACTORY_GATEWAY,
                            FACTORY_SUBNET,
                            FACTORY_HADDR,
                            GRID_X_LEN, GRID_Y_LEN, GRID_Z_LEN,
                            0, 0};      /* Task defaults are to disable it. */
    Position max;
    Task    task;

    uint8_t rtc_sec;
    rtc_read(0, &rtc_sec, 1);

    /* If bit #RTC_CH is set, then the clock is not running. The implementation
    * never stops the RTC from running, so it may just be the battery supply was
    * removed to reset to factory (default) settings. */
    if(bit_is_set(rtc_sec, RTC_CH)) {

        /* Write factory settings into the RTC memory. */
        rtc_write(RTC_BASE, settings, SYS_SIZE);

    /* Load battery-backed (RTC) settings. */
    } else {

        rtc_read(RTC_BASE, settings, SYS_SIZE);
    }

    max.x = settings[SYS_MTR_MAX_X - RTC_BASE];
    max.y = settings[SYS_MTR_MAX_Y - RTC_BASE];
    max.z = settings[SYS_MTR_MAX_Z - RTC_BASE];

    task.interval   =  settings[SYS_TASK_INT    - RTC_BASE];
    task.samples    =  settings[SYS_TASK_SAMPL  - RTC_BASE];

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

    /* Setup #HTTP_SOCKET for HTTP (TCP on port #HTTP_PORT). */
    net_socket_open(HTTP_SOCKET, NET_Sn_MR_TCP, HTTP_PORT);
    net_write8(NET_Sn_CR(HTTP_SOCKET), NET_Sn_CR_LISTEN);

    /* Other modules; complementary ones, first. */
    rsrc_init();
    srvr_init();
    log_init();
    task_init();
    task_set(&task);

    motor_init();

    /* Set operating range *after* the motors have been initialised. */
    motor_set_max(&max);
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

#ifdef ENABLE_SERIAL_IO

ISR(USART_RX_vect) {
    #ifndef ENABLE_DEBUG
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

    fls_wait_WIP();
    #endif /* ! defined (ENABLE_DEBUG) */
}

int usart_putchar(char c, FILE* stream) {
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0        = c;
    loop_until_bit_is_set(UCSR0A, UDRE0);
    return 0;
}

int usart_getchar(FILE* stream) {
    loop_until_bit_is_set(UCSR0A, RXC0);
    return UDR0;
}

void init_usart() {
    /* Set UBRRn value. */
    UBRR0H      = (unsigned char)(UBRR_VALUE>>8);
    UBRR0L      = (unsigned char)(UBRR_VALUE);

    /* Set character size to 8 bits. */
    UCSR0C      = _BV(UCSZ01) | _BV(UCSZ00);

    /* Enable Rx-complete interrupts, Receiver and Transmitter. */
    UCSR0B      = _BV(RXCIE0) | _BV(RXEN0);

    #ifdef ENABLE_DEBUG
    UCSR0B     |= _BV(TXEN0);
    #endif
}
#endif /* defined (ENABLE_SERIAL_IO) */
