/**
* @file
* @brief General utility functions.
*/

#ifndef UTIL_H_INCL
#define UTIL_H_INCL

#include <inttypes.h>

/**
* @brief Convert an unsigned integer to string.
*
* Note that @p buf is the *last* address to write to; the actual array should
* expand before it.
*
* @param[out] buf The array into which to store the result. The address provided
*   is the *last* address that will be used in the conversion (and which will
*   contain @c \0. The array should be large enough to contain the result
*   (inclusive of null-byte).
* @param[in] number The number to convert into string.
* @returns The amount of digits written (null-byte not included).
*/
uint8_t int_to_str(uint8_t* buf, uint16_t number);

#endif /* UTIL_H_INCL */
