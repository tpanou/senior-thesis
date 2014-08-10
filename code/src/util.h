/**
* @file
* @brief General utility functions.
*/

#ifndef UTIL_H_INCL
#define UTIL_H_INCL

#include "rtc.h"
#include "defs.h"

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
uint8_t uint_to_str(uint8_t* buf, uint16_t number);

/**
* @brief Convert a temperature reading into a string.
*
* @p t is expected to contain the integral part of the number in the 7 most
* significant bits. Bit @c 0 is the fraction @f$ 2^{-1} @f$. For more
* information, see #LogRecord.
*
* @param[out] buf The result of the conversion (null-terminated).
* @param[in] len Size of @p buf. Although specified, @p buf should be large
*   enough to accommodate the result, regardless.
* @param[in] t The value to convert.
* @returns The amount of bytes written (non-inclusive of null-byte).
*/
uint8_t temp_to_str(uint8_t* buf, uint8_t len, uint8_t t);

/**
* @brief Convert and IP address string into four bytes.
*
* Uses <stdlib.h>strtol().
*
* @param[out] ip An array of four bytes to write to.
* @param[in] buf An IP address string (null-terminated).
* @returns @c 0, if four, dot-separated, numbers have been parsed; non-zero,
*   otherwise.
*/
uint8_t str_to_inet(uint8_t* ip, uint8_t* buf);

/**
* @brief Convert an array of integers into a string, each separated with a dot.
*
* It receives an array of four bytes and converts them to an equivalent IP
* address string (null-terminated).
*
* More specifically, for each address byte, an initial position is estimated and
* then incremented depending on the number of its digits. Then, a character is
* passed into @p buf for each of its digits (using modulo and quotient).
*
* @param[out] buf The string array to write to.
* @param[in] ip A four-byte array of an IP address (or mask).
* @returns The number of bytes written into @p buf (non-inclusive of null-byte).
*/
uint8_t inet_to_str(uint8_t* buf, uint8_t* ip);


/**
* @brief Read the current date and time from the RTC.
*
* @param[out] dt The current date and time.
* @param[out] day Day of week; @c 1 denotes Sunday.
*/
void get_date(BCDDate* dt, uint8_t* day);

/**
* @brief Set the current date and time of the RTC.
*
* @param[in] dt The current date and time.
* @param[in] day Day of week; @c 1 denotes Sunday.
*/
void set_date(BCDDate* dt, uint8_t day);

/**
* @brief Read string into an #RTCMap variable.
*
* Currently, the string is parsed up to seconds (not including fraction).
* Note that this function does not check the validity of the date as a whole
* (eg, days of month), but rather, that each value does not exceed a maximum
* allowed value.
*
* @param[out] dt
* @param[in] buf String in ISO8601 format (YYYY-MM-DDTHH:mm:ss.sssZ).
* @returns @c 0, if parsing the date was successful; @c 0, otherwise.
*/
int8_t str_to_date(BCDDate* dt, uint8_t* buf);

/**
* @brief Convert the supplied date and time into an ISO8601-formatted string.
*
* Although <stdio.h>sprintf() could be used to format the date, it is chosen not
* to, because this way, it results in smaller code footprint, no need to use the
* stack (for the variable arguments) or store the format string.
*
* @param[out] buf An array, at least 25 bytes wide, that accepts the ISO8601
*   format of the supplied date and time (YYYY-MM-DDTHH:mm:ss.sssZ). The string
*   will be null-terminated. Fraction of a second always reads zero; time-zone
*   is set to @c Z (UTC).
* @param[in] dt Date to convert into string.
*/
void date_to_str(uint8_t* buf, BCDDate* dt);

/**
* @brief Read a number of program memory chunks into @p buf.
*
* Accepts a variable amount of addresses of strings that reside in program
* memory. The strings are loaded into @p buf and the address of the first byte
* of each is placed into @p indices.
*
* Some notes:
*   - The last argument *should* always be @c NULL.
*   - @p indices should be at least as large as the number of string addresses
*       given (excluding @c NULL).
*   - @p buf should be large enough to accommodate all requested strings.
*
* @param[out] indices Array of strings read from program memory.
* @param[out] buf Buffer into which to store the strings read from program
*   memory.
* @param[in] ... Program memory addresses to read strings from. The last one
*   *should* be @c NULL.
* @returns The amount of bytes written. In other words, the starting offset at
*   which further strings may be written, if required.
*/
uint16_t pgm_read_str_array(uint8_t** indices, uint8_t* buf, ...);

/**
* @brief Map an #RTCMap to a #BCDDate variable.
*
* It is used in-line by get_date().
*
* @param[out] dt Conversion destination.
* @param[in] rtc Source.
*/
static inline void rtc_to_date(BCDDate* dt, RTCMap* rtc);

/**
* @brief Map a #BCDDate to an #RTCMap variable.
*
* It is used in-line by set_date().
* Note that @link RTCMap#date date@endlink will not be set.
*
* @param[out] rtc Conversion destination.
* @param[in] dt Source.
*/
static inline void date_to_rtc(RTCMap* rtc, BCDDate* dt);

#endif /* UTIL_H_INCL */
