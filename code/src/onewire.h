/**
* @file
* @addtogroup 1_wire 1-wire bus
* @{
*
* @brief Single drop 1-wire bus interface.
*
* The current implementation provides functions to reset, read and write the DQ
* line. No device negotiation is implemented (ROM Search).
*/

#ifndef ONEWIRE_H_INCL
#define ONEWIRE_H_INCL

#include "defs.h"

#include <avr/io.h>
#include <util/delay.h>

#include <inttypes.h>

/**
* @brief ROM command: access memory functions without supplying device code.
*/
#define W1_ROM_SKIP             0xCC

/**
* @brief Memory command: start reading scratchpad memory contents.
*/
#define W1_READ_SCRATCHPAD      0xBE

/**
* @brief Memory command: begin temperature conversion.
*/
#define W1_CONVERT_T            0x44

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

/**
* @brief Send a data byte over the 1-wire DQ line.
*
* Upon completion, the DQ bus is ready to be operated upon immediately.
*
* @param[in] byte Data to send.
*/
void w1_write(uint8_t byte);

/**
* @brief Read up to 16 bits from the 1-wire DQ line.
*
* @param[in] bits Amount of bits to read (up to 16).
* @return Data read from DQ line.
*/
uint16_t w1_read(uint8_t bits);

#endif /* ONEWIRE_H_INCL */
/** @} */
