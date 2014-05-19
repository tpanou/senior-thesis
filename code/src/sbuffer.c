
#include "sbuffer.h"
#include <stdio.h>

static int8_t s_update() {
    uint16_t rx_size = getSn_RX_RSR(buf_Sn); /* Amount of available data. */
    uint16_t fragment; /* Actual amount of bytes to be read. */

    /* Read a chunk from the available data up to a maximum of @c NET_BUF_SIZE
    * and fit it into @c buf. */
    if(rx_size > 0) {

        /* @c buf may not be completely empty at the time of invocation. That
        * amount of data is represented by @c buf_data and conserve space until
        * read (ie, @buf_RD is promoted over them). */
        if(rx_size <= NET_BUF_LEN - buf_data) {
            fragment =  rx_size;
        } else {
            fragment = NET_BUF_LEN - buf_data;
        }

        /* Depending on the state of the RD/WR offsets, data may not be able to
        * be written in one contiguous block but, rather, wrapped around the end
        * of the buffer. */
        if(buf_WR + fragment > NET_BUF_LEN) {
            uint16_t bound = NET_BUF_LEN - buf_WR;

            recv(buf_Sn, &(buf[buf_WR]), bound); /* Read up to buffer limit. */

            /* Read the remainder of bytes. */
            recv(buf_Sn, buf, fragment - bound);
            buf_WR  = fragment - bound;     /* Update WR offset. */
        } else {
            recv(buf_Sn, &(buf[buf_WR]), fragment);
            buf_WR += fragment; /* Update WR offset. */
        }

        buf_data += fragment;   /* Update the amount of in-buffer data. */
    } else {
        return EOF;
    }
    return 0;
}
