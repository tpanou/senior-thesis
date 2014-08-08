/**
* @file
* @addtogroup log Measurement Log
* @brief Manage measurement logging.
*
* The records are stored within EEPROM in a circular structure that,
* essentially, is a simplified circular buffer. Its contents span from address
* #LOG_BASE_ADDR for a total of #LOG_LEN records. The size and contents of each
* record is determined by #LogRecord.
*
* The chosen structure permits adding new records that, once the available space
* has been depleted, replace the oldest ones. To achieve this, the offset of the
* oldest record and the current amount of records are maintained (see #log).
* Obviously, this offset is incremented as older records are being replaced.
* Offsets that are calculated based on this start offset are referred to as
* 'physical offsets' because they may be used to obtain the physical address of
* a particular record.
*
* To avoid the implementation specifics and the manipulation of a circular
* structure in higher abstraction functions, such as log_get_set() and
* log_get_next(), they are designed to treat the Log as a linear structure where
* the record at offset @c 0 is always the oldest one and the one at
* (@link #log log.count@endlink - 1), the newest. These offsets are referred to
* as 'logical offsets' and must first be mapped to a physical one before
* accessing the corresponding record. This mapping is achieved by
* log_get_offset().
*
* @{
*/

#ifndef LOG_H_INCL
#define LOG_H_INCL

#include "defs.h"

#include <inttypes.h>

/**
* @brief The amount of total records to store in the EEPROM Log.
*/
#define LOG_LEN             90

/**
* @brief The EEPROM address to start storing log records.
*/
#define LOG_BASE_ADDR       34

/**
* @brief Shorthand to calculate the address of a physical offset.
*/
#define LOG_ADDR(offset)   (LOG_BASE_ADDR + offset*sizeof(LogRecord))

/**
* @brief Index of a single record and the total amount of records.
*
* This is returned by log_get_set() and may be used with log_get_next() to
* access the available Log records or a subset of them.
*
* It is also used as a reference point for the state of the circular Log
* structure. See #log.
*/
typedef struct {
    /** @brief Index of some record.
    *
    * It spans from @c 0 up to #LOG_LEN - 1.
    */
    uint8_t index;

    /** @brief Amount of records. */
    uint8_t count;
} LogRecordSet;

/**
* @brief Record structure.
*
* Note that the first member must be #BCDDate. This helps to avoid loading a
* record in its entirety when searching for a particular date.
*/
typedef struct {
    /** @brief Date of record. Must be unique among all records. */
    BCDDate     date;

    /** @brief Abscissa of sample coordinates. */
    uint8_t     x;

    /** @brief Ordinate of sample coordinates. */
    uint8_t     y;

    /** @brief Temperature of sample. */
    uint8_t     t;

    /** @brief Relative humidity of sample. */
    uint8_t     rh;

    /** @brief pH of sample. */
    uint8_t     ph;
} LogRecord;

/**
* @brief Initialise Log dependencies.
*
* It loads @c index and @c count from EEPROM into #log.
*/
void log_init();

/**
* @brief Add a new log record.
*
* Apart from writing the record, it also updates @c index, @c count (in EEPROM)
* and #log, as needed.
*
* @param[in] rec The record to append to the log.
*/
void log_append(LogRecord* rec);

/**
* @brief Read the next record found in the record @p set.
*
* Each successive call to this function reads one more record into @p rec. @c -1
* is returned when there are no more records available.
*
* A valid #LogRecordSet is obtained via log_get_set(). As @p set is modified
* internally, any external modification could cause unexpected behaviour and
* should be avoided.
*
* @param[in] rec Contents of the next record.
* @param[in,out] set Set of records to return.
* @returns @c 0, if a record has been loaded into @p rec; @c -1, if there are no
*   more records available (in which case, the contents of @p rec are *not*
*   updated).
*/
uint8_t log_get_next(LogRecord* rec, LogRecordSet* set);

/**
* @brief Translate a logical to a physical offset.
*
* Physical offsets are used to access a record within the storage structure.
* Logical offsets refer to a conceptual array where the oldest record is always
* found at offset @c 0. This function translates a logical to a physical offset.
*
* @param[in] index A value between @c 0 and #LOG_LEN - 1. Otherwise, an offset
*   that lies outside the storage may be returned.
* @returns The physical offset that may be used to access a record.
*/
static uint8_t log_get_offset(uint8_t index);

#endif /* LOG_H_INCL */
/** @} */
