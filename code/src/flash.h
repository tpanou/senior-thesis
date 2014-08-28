
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

#endif /* FLASH_H_INCL */
