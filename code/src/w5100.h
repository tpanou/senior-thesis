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

#endif /* W5100_H_INCL */
/** @} */
