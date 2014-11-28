/**
* @file
*/

#ifndef SENSOR_H_INCL
#define SENSOR_H_INCL

#include "defs.h"

#include <inttypes.h>

/**
* @brief Activates access to the 1-wire DQ line.
*
* Sets the appropriate signals so that the 1-wire DQ line may be access through
* #MUX_2Z pin. It also enables the multiplexer it is connected to.
*
* Note that the 1-wire DQ may not be access at the same time the motors are
* being operated.
*/
#define W1_ENABLE()                 \
MUX_S0_PORT        |=  _BV(MUX_S0); \
MUX_S1_PORT        |=  _BV(MUX_S1); \
MUX_ENABLE()

/**
* @brief Disable access to the 1-wire DQ line.
*
* This actually disables the multiplexer. It is advised the DQ line always be
* deactivated this way because other components that share the multiplexer may
* expect it to be disabled by default.
*/
#define W1_DISABLE()                \
MUX_DISABLE()

/**
* @brief Samples the DS18B20 digital thermometer and returns its reading.
*
* This function expects the DS18B20 to be the only slave connected to the 1-wire
* DQ line. It returns the reading in a 16-bit, sign-extended two's complement
* format, as follows:
*   - Bits @c 15--11 contain the sign.
*   - Bits @c 10--4 the integral part of the reading (@f$2^{6}-2^{0}@f$).
*   - Bits @c 3--0 the fraction of the reading (@f$2^{-1}-2^{-4}@f$).
*
* @returns The temperature reading of the sensor.
*/
uint16_t sens_read_t();

#endif /* SENSOR_H_INCL */
