
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

#endif /* FLASH_H_INCL */
