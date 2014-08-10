/**
* @file
*/

#ifndef RTC_H_INCL
#define RTC_H_INCL

#include <inttypes.h>

/**
* @brief Structure of the  DS1307 RTC memory map.
*
* These registers are both readable and writable and internally buffered. All
* values are in BCD format. *DS1307 p.8*
*/
typedef struct {

    /** @brief Seconds elapsed (00-59).
    *
    * Bit @c 7 corresponds to #RTC_CH.
    */
    uint8_t sec;

    /** @brief Minutes elapsed (00-59). *DS1307 p.8* */
    uint8_t min;

    /** @brief Hours elapsed (01-12+AM/PM or 00-23).
    *
    * Bit @c 6 corresponds to #RTC_HMODE; bit @c to #RTC_AM_PM.
    */
    uint8_t hour;

    /** @brief Day of week (01-07).
    *
    * Any day may be specified as the first day of the week. *DS1307 p.8*
    */
    uint8_t day;

    /** @brief Date (01-31). *DS1307 p.8* */
    uint8_t date;

    /** @brief Month (01-12). *DS1307 p.8* */
    uint8_t mon;

    /** @brief Year (00-99). *DS1307 p.8* */
    uint8_t year;

    /** @brief Controls the output on pin @c SQW/OUT.
    *
    * - Bit @c 7 corresponds to bit #RTC_SQW_OUT.
    * - Bit @c 4 corresponds to bit #RTC_SQWE.
    * - Bits @c 1 and @c 0 correspond to #RTC_RS1 and #RTC_RS0, respectively.
    */
    uint8_t sqw;
} RTCMap;

/**
* @brief Address of DS1307 on TWI bus.
*
* The address of the DS1307 is @c 1101000 (*DS1307 p.12*). When the address (7
* bits long) is transmitted over the bus, it is followed by the operation bit
* (@c\f$R/\overline{W}\f$). The address is shifted one time to the left so it
* can facilitate adding the operation bit on the LSB of that data byte.
*/
#define RTC_ADDR       (0xD0)

/**
* @brief Enables or disables the timekeeping operation (Clock Halt - CH).
*
* Setting this bit results in stopping the RTC and consumption is further
* reduced. Clearing it enables the oscillator. Upon first power application, it
* defaults to @c 1. *DS1307 p.8*
*/
#define RTC_CH          7

/**
* @brief Chooses between 12- and 24-hour mode.
*
* @c 1 is for 12-hour mode in which case, bit #RTC_AM_PM corresponds to AM/PM;
* otherwise, it is part of the ten-hours BCD digit. *DS1307 p.8*
*/
#define RTC_HMODE       6

/**
* @brief AM/PM or ten-hours of BCD digit.
*
* If bit @c RTC_HMODE is set, then 12-hour mode of operation is selected; this
* bit corresponds to AM/PM, @c 0 denoting AM. If in 24-hour mode, this bit is
* part of BCD hour value.
*/
#define RTC_AP_MP       5

/**
* @brief Determines the constant output on pin @c SQW/OUT.
*
* When bit #RTC_SQWE is cleared (ie, the square-wave output is disabled),
* if this bit is cleared as well, @c SQW/OUT is set to low; otherwise, it is set
* to high.
*/
#define RTC_SQW_OUT     7

/**
* @brief Enables or disables waveform generation.
*
* If set, a square-wave is generated on pin @c SQW/OUT of the frequency
* determined by bits #RTC_RS1 and #RTC_RS0.
*/
#define RTC_SQWE        4

/**
* @brief Determines, along with #RTC_RS0, the frequency of the square-wave
* output.
*
* For details, see #RTC_RS0.
*/
#define RTC_RS1         1

/**
* @brief Determines, along with #RTC_RS1, the frequency of the square-wave
* output.
*
*
* To enable the square-wave output on pin @c SQW/OUT, bit #RTC_SQWE must be set.
* The value of RTC_RS[1:0] control the generated wave frequency as follows:
*   - @c 0 outputs 1Hz.
*   - @c 1 outputs 4.096kHz.
*   - @c 2 outputs 8.192kHz.
*   - @c 3 outputs 32.768kHz.
*/
#define RTC_RS0         0

/**
* @brief Set the RTC time.
*
* @param[in] rtc The bytes to send to the RTC. If @c -1 is returned, not all
* bytes may have been sent to the RTC.
* @returns @c 0 on success; @c -1, otherwise.
*/
int8_t rtc_set(RTCMap* rtc);

/**
* @brief Get the RTC time.
*
* Updates the member of @p rtc to reflect the RTC values.
*
* @param[out] rtc The bytes received from the RTC. If @c -1 is returned, the
* contents of @p rtc may be partially updated.
* @returns @c 0 on success; @c -1, otherwise.
*/
int8_t rtc_get(RTCMap* rtc);

/**
* @brief Reset the DS1307 register pointer back to the first address.
*
* The DS1307 utilizes an internal register pointer for all read and write
* operations so that each time a byte is sent or received, the pointer is
* automatically incremented to point to the memory address (*DS1307 p.12*). This
* function helps to reset this register pointer back to @c 0.
*
* Note, this function does not release the TWI bus upon successful pointer
* initialization but only in the event of a failure.
*
* @returns @c 0 on success; @c -1, otherwise.
*/
static int8_t rtc_reset_pointer();

#endif /* RTC_H_INCL */
