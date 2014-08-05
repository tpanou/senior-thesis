/**
* @file
* @addtogroup 1_wire 1-wire bus
*/

#ifndef ONEWIRE_H_INCL
#define ONEWIRE_H_INCL

#include "defs.h"

#include <avr/io.h>
#include <util/delay.h>

#include <inttypes.h>

/**
* @brief The bit read over 1-wire DQ line.
*
* An appropriate read slot should be initiated before reading this value.
*
* @returns @c 0, if the slave transmitted a @c 0; non-zero, otherwise.
*/
#define W1_READ()                   \
(W1_DQ_PIN & _BV(W1_DQ))

/**
* @brief Pull 1-wire DQ low.
*/
#define W1_DQ_LOW()                 \
W1_DQ_PORT         &= ~_BV(W1_DQ);  \
W1_DQ_DDR          |=  _BV(W1_DQ)

/**
* @brief Release 1-wire so it may rise to high via the external resitor.
*/
#define W1_DQ_RELEASE()             \
W1_DQ_DDR          &= ~_BV(W1_DQ)

/**
* @brief Initialise the 1-wire DQ bus (reset and presence pulses).
*
* Upon completion, the DQ bus is ready to be operated upon immediately.
*
* @returns @c 0, if a presence pulse was detected; non-zero, otherwise.
*/
uint8_t w1_reset();

#endif /* ONEWIRE_H_INCL */
