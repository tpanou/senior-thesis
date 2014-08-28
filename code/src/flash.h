
#ifndef FLASH_H_INCL
#define FLASH_H_INCL

#include "defs.h"

#include <inttypes.h>

/**
* @brief Read flash, beginning at the specified address.
*/
#define FLS_READ        0x03

/**
* @brief Write flash, beginning at the specified address.
*/
#define FLS_WRITE       0x02

/**
* @brief Set the write-enable latch.
*/
#define FLS_WREN        0x06

/**
* @brief Reset the write-enable latch.
*/
#define FLS_WRDI        0x04

/**
* @brief Read Status Register.
*/
#define FLS_RDSR        0x05

/**
* @brief Write Status Register.
*/
#define FLS_WRSR        0x01

/**
* @brief Erase page the specified address belongs to.
*/
#define FLS_PE          0x42

/**
* @brief Erase sector the specified address belongs to.
*/
#define FLS_SE          0xD8

/**
* @brief Erase chip.
*/
#define FLS_CE          0xC7

/**
* @brief Wake from Deep power-down mode and return device signature.
*/
#define FLS_RDIP        0xAB

/**
* @brief Read flash, beginning at the specified address.
*/
#define FLS_DPD         0xB9

/**
* @brief Prepare the SPI bus to communicate with the Flash.
*
* This function:
*   - Disables SPI (in case it was running).
*   - Sets up the appropriate clock rate (see #FLS_SPSR and #FLS_SPCR).
*   - Enables the chip (see #FLS_ENABLE()).
*   - Delays 1us for @c nCS setup time (T_{CSS}, *25LC1024 p.3*).
*   - Does *not* enable the SPI clock!
*/
void fls_select();

/**
* @brief Terminate communication with the Flash.
*
* This function:
*   - Delays 1us for @c nCS hold time (T_{CSH}, *25LC1024 p.3*).
*   - Disables the chip (see #FLS_DISABLE()).
*   - Delays 1us for MISO output disable time (T_{DIS}, *25LC1024 p.4*).
*/
void fls_deselect();

/**
* @brief Send the specified command, optionally receiving/sending data.
*
* This function, in terms of the 25LC1024 instruction set, may be used to send a
* command:
*   - with no additional payload,
*   - followed by a single byte,
*   - and read one byte from the chip.
*
* To send/read multiple bytes to/from a specific address range, use
* fls_exchange().
*
* Once completed, the @c nCS will be pulled high.
*
* @param[in] c The command to send.
* @param[in,out] data If not @c NULL, the contents of the address are sent to
*   the Flash and the response is written it.
*/
void fls_command(uint8_t c, uint8_t* data);

/**
* @brief
*
* It sends command @p c followed by an address (calculated from @p page). It may
* optionally send *and* receive @p len bytes. Note that this function
* *exchanges* bytes and, so, it *always* sends @p len bytes from @p buf and
* writes that many bytes back into it.
*
* Note that this function accepts the page to start reading/writing from/to and
* *not* an address, implying that each exchange begins at the first byte of any
* page. Also note that the 25LC1024 may not be written data on successive pages
* *at the same time*; on the contrary, the data will be wrapped around to the
* beginning of the page, overwriting any contents (*25LC1024 p.6*); a
* limitation/feature of the chip. Using this function, this typically means,
* that no more than 256 bytes should be exchanged for a @c WRITE operation.
*
* @param[in] c The command to send.
* @param[in] page The page to read from or write to. The 25LC1024 contains
*   1Mbit, organised in 8-bit words. Each page contains 256 bytes, so there are
*   512 pages available (@c 0 through @c 511);
* @param[in,out] buf The bytes to sent. Upon return, it contains the bytes read
*   from the Flash. Note that the contents of this array are *always* altered,
*   even if @c specified an output command!
* @param[in] len Size of bytes to exchange (read/write); @c 0 is valid, in which
*   case, @p buf could be @c NULL.
*/
void fls_exchange(uint8_t c, uint16_t page, uint8_t* buf, uint8_t len);

#endif /* FLASH_H_INCL */
