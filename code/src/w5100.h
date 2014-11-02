/**
* @file
* @brief API for the network module W5100.
* @addtogroup network_w5100 Network module (W5100)
* @{
*
* @brief API for the network module W5100.
*/

#ifndef W5100_H_INCL
#define W5100_H_INCL

#include <inttypes.h>

/**
* @brief Mode register.
*
* Responsible for software reset and enabling ping block, PPPoE, Indirect bus
* auto-increment and Indirect bus mode.
*
* See bits: #NET_MR_RST, #NET_MR_PB.
*/
#define NET_MR              0x0000

/**
* @brief Reset bit; if set, the module will reset.
*
* Automatically cleared, once reset has completed.
*/
#define NET_MR_RST          7

/**
* @brief Ping Block mode bit; if set, ping responses are enabled.
*/
#define NET_MR_PB           4

/* There are more bits specified which are irrelevant to the implementation. */

/**
* @brief Default Gateway Address Register.
*
* 4-Bytes long.
*/
#define NET_GAR             0x0001

/**
* @brief Subnet Mask Register.
*
* 4-Bytes long.
*/
#define NET_SUBR            0x0005

/**
* @brief Source Hardware Address Register.
*
* 6-Bytes long.
*/
#define NET_SHAR            0x0009

/**
* @brief Source IP Address Register.
*
* 4-Bytes long.
*/
#define NET_SIPR            0x000F

/**
* @brief Interrupt Register (read-only).
*
* Contains interrupt flags. Pin @c nINT remains low for as long as there is at
* least one such bit set, granted that its mask bit is also set (see #NET_IMR).
*
* To set which events cause an interrupt, use #NET_IMR.
*/
#define NET_IR              0x0015

/**
* @brief Interrupt Mask Register.
*
* Specifies under which circumstances an interrupt is generated. Each bit in
* this register corresponds to the interrupt flag bit of #NET_IR at the same
* position.
*
* See bits: #NET_IR_S0
*/
#define NET_IMR             0x0016

/**
* @brief Interrupt coming from Socket n.
*
* @param[in] n Socket to produce interrupt (@c 0--@c 3).
*/
#define NET_IR_Sn(n)       (1<<n)

/* There are more bits specified which are irrelevant to the implementation. */

/**
* @brief Rx Memory Size Register; default 0x55.
*
* Distributes the available buffer for incoming data to the specified sockets.
* The total memory is 8KB. Size of Socket 0 is determined by bits @c 0 and @c 1,
* and so on.
*
* See #NET_SIZEn().
*/
#define NET_RMSR            0x001A

/**
* @brief Tx Memory Size Register; default 0x55.
*
* Distributes the available buffer for outgoing data to the specified sockets.
* The total memory is 8KB. Size of Socket 0 is determined by bits @c 0 and @c 1,
* and so on.
*
* See #NET_SIZEn().
*/
#define NET_TMSR            0x001B

/**
* @brief #NET_TMSR and #NET_RMSR combination of 1KB.
*
* To create the appropriate value for a specific Socket, use #NRW_SIZEn().
*/
#define NET_SIZE_1          0x00

/**
* @brief #NET_TMSR and #NET_RMSR combination of 2KB.
*
* To create the appropriate value for a specific Socket, use #NRW_SIZEn().
*/
#define NET_SIZE_2          0x01

/**
* @brief #NET_TMSR and #NET_RMSR combination of 4KB.
*
* To create the appropriate value for a specific Socket, use #NRW_SIZEn().
*/
#define NET_SIZE_4          0x02

/**
* @brief #NET_TMSR and #NET_RMSR combination of 8KB.
*
* To create the appropriate value for a specific Socket, use #NRW_SIZEn().
*/
#define NET_SIZE_8          0x03

/**
* @brief Convenience macro to create Socket size bits.
*
* @param[in] n Socket number (@c 0--@c 3).
* @param[in] x Socket size (one of #NET_SIZE_1, #NET_SIZE_2, #NET_SIZE_4,
*   #NET_SIZE_8).
*/
#define NET_SIZEn(n, x)    (x << (n*2))

/**
* @brief The start address of the Tx buffer.
*/
#define NET_TX_BASE 0x4000

/**
* @brief The start address of the Rx buffer.
*/
#define NET_RX_BASE 0x6000

/**
* @brief Base address offset of a socket.
*
* Each Socket is controlled by a set amount of registers. They span for @c 0x100
* (@c 256) bytes starting at a *base* address. For instance, Socket 0 starts at
* @c 0x400, Socket 1 starts at @c 0x500, and so on. This helps calculate the
* base address for the specified Socket.
*
* @param[in] n Socket to calculate its base address (@c 0--@c 3).
*/
#define NET_Sn_OFFSET(n)   (n*0x100 + 0x400)

/**
* @brief Socket Mode Register.
*
* Controls the operation of this Socket.
*
* See: #NET_Sn_MR_CLOSED, #NET_Sn_MR_TCP.
*/
#define NET_Sn_MR(n)       (0x00 + NET_Sn_OFFSET(n))

/**
* @brief Disable this Socket.
*
* Used with #NET_Sn_MR().
*/
#define NET_Sn_MR_CLOSED   (0x0)

/**
* @brief Use Socket in TCP mode.
*
* Used with #NET_Sn_MR().
*/
#define NET_Sn_MR_TCP      (0x1)

/**
* @brief Socket Command Register.
*
* Accepts operation command to perform.
*
* See: #NET_Sn_CR_OPEN, #NET_Sn_CR_LISTEN, #NET_Sn_CR_CONNECT,
* #NET_Sn_CR_DISCON, #NET_Sn_CR_CLOSE, #NET_Sn_CR_SEND, #NET_Sn_CR_RECV.
*/
#define NET_Sn_CR(n)       (0x01 + NET_Sn_OFFSET(n))

/**
* @brief Initialise Socket.
*
* Used with NET_Sn_CR().
*/
#define NET_Sn_CR_OPEN      0x01

/**
* @brief Listen for incoming connection requests (TCP mode).
*
* Used with NET_Sn_CR().
*/
#define NET_Sn_CR_LISTEN    0x02

/**
* @brief Connect to remote host (TCP mode).
*
* Used with NET_Sn_CR().
*/
#define NET_Sn_CR_CONNECT   0x03

/**
* @brief Send connection termination request (TCP mode).
*
* Used with NET_Sn_CR().
*/
#define NET_Sn_CR_DISCON    0x08

/**
* @brief Close Socket (change value of #NET_Sn_SR() to #NET_SN_SR_CLOSED).
*
* Used with NET_Sn_CR().
*/
#define NET_Sn_CR_CLOSE     0x10

/**
* @brief Notify to send data from the outgoing buffer of the Socket.
*
* Used with NET_Sn_CR().
*/
#define NET_Sn_CR_SEND      0x20

/**
* @brief Notify that data have been read from the incoming buffer of the Socket.
*
* Used with NET_Sn_CR().
*/
#define NET_Sn_CR_RECV      0x40

/**
* @brief Interrupt flags of this Socket.
*
* Each bit specifies a particular condition. See: #NET_Sn_IR_SEND_OK,
* #NET_Sn_IR_TIMEOUT, #NET_Sn_IR_RECV, #NET_Sn_IR_DISCON, #NET_Sn_IR_CON.
*/
#define NET_Sn_IR(n)       (0x02 + NET_Sn_OFFSET(n))

/**
* @brief Transmission is completed.
*
* Used with #NET_Sn_IR().
*/
#define NET_Sn_IR_SEND_OK   4

/**
* @brief Time-out (of connection or data transmission).
*
* Used with #NET_Sn_IR().
*/
#define NET_Sn_IR_TIMEOUT   3

/**
* @brief Available data.
*
* This bit remains set for as long as there data available for this Socket (even
* after executing #NET_Sn_CR_RECV).
*
* Used with #NET_Sn_IR().
*/
#define NET_Sn_IR_RECV      2

/**
* @brief Connection termination is requested or completed.
*
* Used with #NET_Sn_IR().
*/
#define NET_Sn_IR_DISCON    1

/**
* @brief Connection established.
*
* Used with #NET_Sn_IR().
*/
#define NET_Sn_IR_CON       0

/**
* @brief Status flags of this Socket.
*
* Each bit specifies a particular state. See: #NET_Sn_SR_CLOSED,
* #NET_Sn_SR_INIT, #NET_Sn_SR_LISTEN, #NET_Sn_SR_ESTAB, #NET_Sn_SR_CLOSEWAIT.
*/
#define NET_Sn_SR(n)       (0x03 + NET_Sn_OFFSET(n))

/**
* @brief Connection is terminated.
*
* Used with NET_Sn_SR(). See *w5100 p.29*.
*/
#define NET_Sn_SR_CLOSED    0x00

/**
* @brief This occurs after #NET_Sn_CR_OPEN is given.
*
* Used with NET_Sn_SR(). See *w5100 p.30*.
*/
#define NET_Sn_SR_INIT      0x13

/**
* @brief Socket is listening for incoming connections.
*
* Used with NET_Sn_SR(). See *w5100 p.30*.
*/
#define NET_Sn_SR_LISTEN    0x14

/**
* @brief Connection established; data may now be received and sent.
*
* Used with NET_Sn_SR(). See *w5100 p.30*.
*/
#define NET_Sn_SR_ESTAB     0x17

/**
* @brief Termination request has been received.
*
* Used with NET_Sn_SR(). See *w5100 p.30*.
*/
#define NET_Sn_SR_CLOSEWAIT 0x1C

/**
* @brief Port number of this Socket.
*
* 2-Bytes long.
*
* Each bit specifies a particular state. See: #NET_Sn_SR_CLOSED,
* #NET_Sn_SR_INIT, #NET_Sn_SR_LISTEN, #NET_Sn_SR_ESTAB, #NET_Sn_SR_CLOSEWAIT.
*/
#define NET_Sn_PORT(n)     (0x04 + NET_Sn_OFFSET(n))

/**
* @brief Socket TX Free Size Register.
*
* 2-Bytes long.
*/
#define NET_Sn_TX_FSR(n)   (0x20 + NET_Sn_OFFSET(n))

/**
* @brief Socket TX Read Pointer Register (read-only).
*
* 2-Bytes long.
*/
#define NET_Sn_TX_RR(n)    (0x22 + NET_Sn_OFFSET(n))

/**
* @brief Socket TX Write Pointer Register.
*
* 2-Bytes long.
*/
#define NET_Sn_TX_WR(n)    (0x24 + NET_Sn_OFFSET(n))

/**
* @brief Socket RX Received Size Register (read-only).
*
* 2-Bytes long.
*/
#define NET_Sn_RX_RSR(n)   (0x26 + NET_Sn_OFFSET(n))

/**
* @brief Socket RX Read Pointer Register.
*
* 2-Bytes long.
*/
#define NET_Sn_RX_RR(n)    (0x28 + NET_Sn_OFFSET(n))

/**
* @brief Initialise Socket buffer sizes.
*
* This should be called at least once before attempting to operate the Sockets.
* Setting #NET_RMSR and #NET_TMSR separately is not required. Calling this
* function should be preferred, instead, because it also initialises some
* internal settings.
*
* Care should be when allocating more than 2KB to a Socket; the total available
* memory for either Tx and Rx buffers is 8KB; *shared by all* sockets.
*
* Also, see #NET_SIZEn().
*
* @param[in] Value of #NET_TMSR.
* @param[in] Value of #NET_RMSR.
*/
void net_socket_init(uint8_t tx, uint8_t rx);

/**
* @brief Initialises a Socket.
*
* @param[in] s The Socket to initialise (@c 0--@c 3).
* @param[in] mode The mode of operation of @p s. See #NET_Sn_MR().
* @param[in] port The port number Socket @p s should operate on.
*/
void net_socket_open(uint8_t s, uint8_t mode, uint16_t port);

/**
* @brief Prepare the SPI bus to communicate with the W5100.
*
* This function:
*   - Disables SPI (in case it was running).
*   - Sets up the appropriate clock rate (see #NET_SPSR and #NET_SPCR).
*   - Enables the chip (see #NET_ENABLE()).
*   - Delays 1us for @c nCS setup time (*W5100 p.67*).
*   - Does *not* enable the SPI clock!
*/
void net_select();

/**
* @brief Convenience function to send a byte over to the W5100.
*
* @param[in] addr The address to write to.
* @param[in] data The data to send.
*/
void inline net_write8(uint16_t addr, uint8_t data);

/**
* @brief Convenience function to read a byte over from the W5100.
*
* @param[in] addr The address to read from.
* @returns The data read.
*/
uint8_t net_read8(uint16_t addr);

/**
* @brief Convenience function to read a word over from the W5100.
*
* @param[in] addr The address to start reading from.
* @retuns The data to read.
*/
uint16_t net_read16(uint16_t addr);

/**
* @brief Convenience function to send a word over to the W5100.
*
* @param[in] addr The address to start writing to.
* @param[in] data The data to send.
*/
void net_write16(uint16_t addr, uint16_t data);

/**
* @brief Wrapper around net_exchange() to send data to the W5100.
*
* It is safe to assume that, upon completion, the contents of @p buf will not
* have been altered.
*
* Also, see net_exchange().
*
* @param[in] addr The address to start writing to. It is incremented for each
*   byte sent.
* @param[in] buf The data to send.
* @param[in] len The amount of bytes to write.
*/
void inline net_write(uint16_t addr, uint8_t* buf, uint16_t len);

/**
* @brief Wrapper around net_exchange() to read data from the W5100.
*
* Also, see net_exchange().
*
* @param[in] addr The address to start reading from. It is incremented for each
*   byte received.
* @param[out] buf The data read.
* @param[in] len The amount of bytes to read.
*/
void inline net_read(uint16_t addr, uint8_t* buf, uint16_t len);

/**
* @brief Exchange the specified amount of bytes starting at @p addr.
*
* It is safe to assume that, if a write command is specified (@c 0xF0), the
* contents of @p buf will not have been altered.
*
* @param[in] c @c 0xF0 to @c write to, @c 0x0F to @c read data from the W5100.
* @param[in] addr The address to start writing/reading to/from. It is
*   incremented for each byte sent/received.
* @param[in,out] buf The data to send. Upon completion, it contains the data
*   received, if @p c was @c 0x0F.
* @param[in] len The amount of bytes to write/read.
*/
void net_exchange(uint8_t c, uint16_t addr, uint8_t* buf, uint16_t len);

/**
* @brief Send data to a W5100 Socket output buffer.
*
* It is safe to call this function with a @p len of @c 0 and @p flush of
* non-zero to sent any previously set W5100 buffer data on socket @p s. In this
* case, @p buf could be @c NULL.
*
* @param[in] s The socket to send data to.
* @param[in] buf Array containing bytes to send. This should be at least @p len
*   bytes long.
* @param[in] The number of bytes to copy from @p buf into the W5100 output
*   buffer.
* @param[in] flush Designates whether data in @buf along with all previously
*   unsent data of socket @p s should be sent out with this call.
* @returns @c The available space in the output buffer of W5100 for socket @c s
*   (after appending @p len bytes from @p buf), if @p flush was @c 0; the socket
*   size (in bytes), if the buffer was just flushed (ie, @p flush was non-zero);
*   a negative number for the amount of bytes that cannot fit into the available
*   space.
*/
uint16_t net_send(uint8_t s, uint8_t* buf, uint16_t len, uint8_t flush);

/**
* @brief Receive data from a W5100 Socket input buffer.
*
* @param[in] s The socket to read data from.
* @param[in] buf Array of bytes read. This should be at least @p len bytes long.
* @param[in] The number of bytes to read from the W5100 buffer.
* @returns @c The available bytes in the input buffer (after reading @p len
*   bytes).
*/
uint16_t net_recv(uint8_t s, uint8_t* buf, uint16_t len);

#endif /* W5100_H_INCL */
/** @} */
