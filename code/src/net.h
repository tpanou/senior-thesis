/**
* @file
*/

#ifndef NET_H_INCL
#define NET_H_INCL

#include <inttypes.h>

/**
* @brief Responsible for dealing with interrupts on #HTTP_SOCKET.
*
* It handles the various TCP states and calls srvr_call() when data are
* available. It is also responsible to reopen the socket, once a connection has
* been terminated.
*
* @param[in] s This Socket (@c 0--@c 3).
* @param[in] status The #NET_Sn_IR value at the time of invocation.
*/
void handle_http_socket(uint8_t s, uint8_t status);

#endif /* NET_H_INCL */
