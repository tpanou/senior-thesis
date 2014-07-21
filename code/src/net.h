/**
* @file
*/

#ifndef NET_H_INCL
#define NET_H_INCL

#include <inttypes.h>

/**
* @brief Responsible for dealing with interrupts at socket 0.
*
* Eventually, it will forward incoming data to the HTTP server.
*/
void socket0_handler(uint8_t status);

#endif /* NET_H_INCL */
