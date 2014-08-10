
#include "rtc.h"
#include "defs.h"
#include "twi.h"

#include <avr/io.h>
#include <inttypes.h>

static int8_t rtc_reset_pointer() {

    /* Send start condition. */
    TWI_ATTEMPT(TWI_START(), TWI_SSTART);

    /* Select RTC in write mode. */
    TWI_ATTEMPT(TWI_SLA_W(RTC_ADDR), TWI_SLA_W_ACK);

    /* Transmit the first DS1307 register address to operate on (address 0). */
    TWDR        = 0;
    TWI_DO_WAIT();

    if(TWI_STATUS() != TWI_DATA_W_ACK) {
        TWI_STOP();
        return -1;
    }

    return 0;
}

int8_t rtc_set(RTCMap* rtc) {
    /* Reset register pointer. */
    if(rtc_reset_pointer()) return -1;

    uint8_t i   = 0;

    /* Send the new values, byte-after-byte. */
    do {
        TWDR    = *((uint8_t*)rtc + i);
        TWI_DO_WAIT();
        ++i;
    } while(i < sizeof(RTCMap) && TWI_STATUS() == TWI_DATA_W_ACK);

    /* Notify end-of-transmission. */
    TWI_STOP();

    /* If an error has occurred, report it. */
    if(TWI_STATUS() != TWI_DATA_W_ACK) return -1;

    return 0;
}

int8_t rtc_get(RTCMap* rtc) {

    /* Set DS1307 pointer to @c 0; the first register. */
    if(rtc_reset_pointer()) return -1;

    /* Then, send a repeated start and read the desired length of bytes. The
    * last byte to be read should be followed by a NACK. *DS1307 p.12* */
    TWI_ATTEMPT(TWI_START(), TWI_RSTART);

    /* Select RTC in read mode. */
    TWI_ATTEMPT(TWI_SLA_R(RTC_ADDR), TWI_SLA_R_ACK);

    uint8_t i   = 0;
    uint8_t byte;

    /* Read @p rtc byte after byte acknowledging every byte except for the last
    * one. */
    do {
        TWI_DO_ACK();
        TWI_ATTEMPT(TWI_WAIT(), TWI_DATA_R_ACK);

        byte    = TWDR;
        *(((uint8_t*)rtc) + i) = byte;

        ++i;
    } while(i < sizeof(RTCMap) - 1);

    /* Read the last byte of @p rtc without acknowledging it. *Atmel p.224*,
    * *DS1307 p.10* */
    TWI_ATTEMPT(TWI_DO_WAIT(), TWI_DATA_R_NACK);

    /* Load the last byte into @p rtc. */
    byte        = TWDR;
    *(((uint8_t*)rtc) + i) = byte;

    /* Release the bus. */
    TWI_STOP();
    return 0;
}
