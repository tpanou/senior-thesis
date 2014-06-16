
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
