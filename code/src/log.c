#include "log.h"

#include <avr/eeprom.h>

/**
* @brief Offset of the first stored record.
*
* It spans from @c 0 up to #LOG_LEN - 1.
*/
static uint8_t log_index EEMEM = 0;

/**
* @brief The amount of stored records.
*/
static uint8_t log_count EEMEM = 0;

/**
* @ingroup log
* @brief Internal Log state.
*
* @link LogRecordSet#index .index@endlink is the offset of the oldest record
* within the circular structure. Due to the nature of the storage structure, the
* oldest record may reside anywhere between @c 0 and #LOG_LEN - 1.
* log_get_offset() uses this member to map a logical offset to a physical one.
*
* @link LogRecordSet#count .count@endlink is the amount of valid Log records.
*/
static LogRecordSet log;

void log_init() {
    log.index   =  eeprom_read_byte(&log_index);
    log.count   =  eeprom_read_byte(&log_count);
}

void log_append(LogRecord* rec) {
    uint8_t     write_offset;   /* Offset from #LOG_BASE_ADDR to write to. */

    /* If the storage if full, replace the oldest record with this one. */
    if(log.count == LOG_LEN) {
        write_offset    =  log.index;

        /* Update the physical offset of the oldest record. */
        log.index       =  log.index == LOG_LEN - 1 ? 0 : log.index + 1;
        eeprom_write_byte(&log_index, log.index);

    } else {
        write_offset    =  log_get_offset(log.index + log.count);

        /* Update the count of available records. */
        ++log.count;
        eeprom_write_byte(&log_count, log.count);
    }

    /* Calculate the physical address that corresponds to @c write_offset and
    * write to it. */
    eeprom_update_block(rec, (void*)LOG_ADDR(write_offset), sizeof(LogRecord));
}

uint8_t log_get_next(LogRecord* rec, LogRecordSet* set) {
    uint8_t     read_offset;

    /* Read the next record provided there is one. */
    if(set->count && set->index < LOG_LEN) {
        read_offset = log_get_offset(set->index);

        eeprom_read_block(rec, (void*)LOG_ADDR(read_offset), sizeof(LogRecord));

        ++(set->index);
        --(set->count);

        return 0;
    }
    return -1;
}

static uint8_t log_get_offset(uint8_t index) {
    uint8_t offset;

    /* The requested item lies within #log.index and LOG_LEN - 1. */
    if(LOG_LEN - log.index > index) {
        offset  =  log.index + index;

    /* The requested item lies within @c 0 and log.index. */
    } else {
        offset  =  index - (LOG_LEN - log.index);
    }

    return offset;
}
