
#include "rtc.h"
#include "defs.h"
#include "twi.h"

#include <avr/io.h>
#include <inttypes.h>

int8_t rtc_set(RTCMap* rtc) {
    return rtc_write(0, (uint8_t*)rtc, sizeof (RTCMap));
}

int8_t rtc_get(RTCMap* rtc) {
    return rtc_read(0, (uint8_t*)rtc, sizeof (RTCMap));
}

int8_t rtc_write(uint8_t addr, uint8_t* buf, uint8_t len) {
    uint8_t i   = 0;

    /* Set register pointer to the appropriate word address. */
    if(rtc_set_pointer(addr)) return -1;

    if(!len) return 0;

    /* Send the new values, byte-after-byte. */
    do {
        TWDR    =  buf[i];
        TWI_DO_WAIT();
        ++i;
    } while(i < len && TWI_STATUS() == TWI_DATA_W_ACK);

    /* Notify end-of-transmission. */
    TWI_STOP();

    /* If an error has occurred, report it. */
    if(TWI_STATUS() != TWI_DATA_W_ACK) return -1;

    return 0;
}

int8_t rtc_read(uint8_t addr, uint8_t* buf, uint8_t len) {
    uint8_t i   = 0;
    uint8_t byte;

    /* Set DS1307 pointer to @p addr; the word address to read. */
    if(rtc_set_pointer(addr)) return -1;

    /* Then, send a repeated start and read the desired length of bytes. The
    * last byte to be read should be followed by a NACK. *DS1307 p.12* */
    TWI_ATTEMPT(TWI_START(), TWI_RSTART);

    /* Select RTC in read mode. */
    TWI_ATTEMPT(TWI_SLA_R(RTC_ADDR), TWI_SLA_R_ACK);

    /* Read @p len bytes acknowledging each except for the last one. */
    do {
        TWI_DO_ACK();
        TWI_ATTEMPT(TWI_WAIT(), TWI_DATA_R_ACK);

        byte    =  TWDR;
        buf[i]  =  byte;

        ++i;
    } while(i < len - 1);

    /* Read the last byte of @p rtc without acknowledging it. *Atmel p.224*,
    * *DS1307 p.10* */
    TWI_ATTEMPT(TWI_DO_WAIT(), TWI_DATA_R_NACK);

    /* Load the last byte into @p rtc. */
    byte        =  TWDR;
    buf[i]      =  byte;

    /* Release the bus. */
    TWI_STOP();
    return 0;
}

static int8_t rtc_set_pointer(uint8_t addr) {
    /* Make sure to re-initialise the internal TWI state machine after a
    * possible power-down. */
    TWI_INIT();

    /* Send start condition. */
    TWI_ATTEMPT(TWI_START(), TWI_SSTART);

    /* Select RTC in write mode. */
    TWI_ATTEMPT(TWI_SLA_W(RTC_ADDR), TWI_SLA_W_ACK);

    /* Transmit the DS1307 register to start operating upon. */
    TWDR        =  addr;
    TWI_DO_WAIT();

    if(TWI_STATUS() != TWI_DATA_W_ACK) {
        TWI_STOP();
        return -1;
    }

    return 0;
}
