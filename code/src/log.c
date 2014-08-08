#include "log.h"

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
