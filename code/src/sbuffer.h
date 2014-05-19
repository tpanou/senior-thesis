/**
* @file
*/

#ifndef SBUFFER_H_INCL
#define SBUFFER_H_INCL

#include "defs.h"
#include <inttypes.h>

/**
* @brief The internal input buffer.
*
* Maintins a fragment of the available network data for immediate access.
*/
static uint8_t buf[NET_BUF_LEN];

/**
* @brief The amount of valid data in #buf.
*/
static uint16_t buf_data = 0;

/**
* @brief The offset within #buf of the next-to-read byte.
*/
static uint16_t buf_RD = 0;

/**
* @brief The offset within #buf of the next-to-write byte.
*/
static uint16_t buf_WR = 0;

/**
* @brief Update the contents of the internal network input buffer.
*
* When this function is invoked, a fragment of the available data on the socket
* specified by set_socket_buf() are read into the internal buffer. The size of
* the fragment is the least amount of bytes between the bytes available on the
* socket and the available space (ie, consumed bytes) of the internal buffer.
*
* If s_update() is invoked when no data are available on the socket, @c EOF is
* returned and the internal buffer remains intact. If s_update() is invoked when
* the internal buffer is full, the function completes successfully although no
* new bytes are actually loaded.
*
* To set the size of the internal input buffer, refer to #NET_BUF_LEN. To set
* input buffering for a particular socket, refer to set_socket_buf().
*
* @returns @c 0 if new data were available; @c EOF on end-of-stream.
*/
static int8_t s_update();

#endif /* SBUFFER_H_INCL */
