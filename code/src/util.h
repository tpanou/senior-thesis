/**
* @file
* @brief General utility functions.
*/

#ifndef UTIL_H_INCL
#define UTIL_H_INCL

#include <inttypes.h>

#ifndef NULL
/**
* Specify that a pointer has not been set to a valid address.
*/
#define NULL 0
#endif

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

/**
* @brief Read a number of program memory chunks into @p buf.
*
* Accepts a variable amount of addresses of strings that reside in program
* memory. The strings are loaded into @p buf and the address of the first byte
* of each is placed into @p indices.
*
* Some notes:
*   - The last argument *should* always be @c NULL.
*   - @p indices should be as large as the number of string addresses given
*       (excluding @c NULL).
*   - @p buf should be large enough to accommodate all requested strings.
*
* @param[out] indices Array of strings read from program memory.
* @param[out] buf Buffer into which to store the strings read from program
*   memory.
* @param[in] ... Program memory addresses to read strings from. The last one
*   *should* be @c NULL.
*/
void pgm_read_str_array(uint8_t** indices, uint8_t* buf, ...);

#endif /* UTIL_H_INCL */
