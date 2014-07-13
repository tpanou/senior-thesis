/**
* @file
* @addtogroup sbuffer Stream Buffer
* @ingroup http_server
* @{
*/

#ifndef SBUFFER_H_INCL
#define SBUFFER_H_INCL

#include "defs.h"
#include <inttypes.h>

/**
* @brief The internal input buffer.
*
* Maintains a fragment of the available network data for immediate access.
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
* @brief The socket to buffer data from.
*/
static uint8_t  buf_Sn = 0;

/**
* @brief Specifies which socket's data are to be buffered locally in the MCU.
*
* Functions s_next(), s_peek() and s_drop() operate on a fragment of the network
* module's data, internally buffered in the SRAM of the MCU. The size of this
* buffer, which is determined by #NET_BUF_LEN and is, typically, (much) smaller
* than the size of the module's input buffer, will need to be updated with
* successive fragments (for details, see s_update()). Updating the internal
* buffer is handled by these functions transparently.
*
* In order to facilitate internal input buffering for any socket (any one at a
* time), the socket the data of which are to be buffered needs to be specified
* using this function before attempting any operations with the aforementioned
* functions (or derivatives, thereof). It is not necessary to call this before
* any other function, but only when switching input from a different socket.
*
* @param[in] s Socket to buffer data from.
*/
void set_socket_buf(uint8_t s);

/**
* @brief Read the next byte from the network input stream.
*
* This function actually reads the next byte from the internal buffer. It causes
* a buffer update (see s_update()) when all bytes have been depleted and a new
* one is requested.
*
* @EOF is returned when no bytes are available even after requesting a buffer
* update.
*
* @params[out] The byte read from stream.
* @returns @c 0 on success; @c EOF on end-of-stream.
*/
int8_t s_next(uint8_t* c);

/**
* @brief Read into @p c the byte at offset @p pos from the current position in
* the stream.
*
* This function does not consume bytes from the stream, ie, the next byte read
* by s_next() is the same as if s_peek() was not invoked at all. It does cause a
* buffer update, if not enough bytes are present in the buffer to perform the
* operation.
*
* @param[out] c The byte read at offset @p pos.
* @param[in] pos The offset to read a byte from.
* @returns 0 on success; @c EOF on end-of-stream.
*/
int8_t s_peek(uint8_t* c, uint16_t pos);

/**
* @brief Discard @p count bytes from the stream.
*
* It causes a buffer update, if not enough bytes are present in the buffer to
* perform the operation. If, even a buffer update, the operation cannot be
* completed at its entirety, it is aborted altogether (ie, no bytes are
* discarded).
*
* @param[in] count The amount of bytes to discard.
* @returns 0 on success; @c EOF on end-of-stream.
*/
int8_t s_drop(uint16_t count);

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
/** @} */
