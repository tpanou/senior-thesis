/**
* @file
*/

#include "http_parser.h"

/**
* @brief HTTP method flag-bits that may be OR-ed together.
*
* There is direct correlation between these bit-flags and METHOD_* macro
* definitions (such as #METHOD_GET and #METHOD_PUT) the latter being used, most
* notably, in #HTTPRequest::method, although they are not the same. Each
* bit-flag is a power of 2 so they may be intermixed to describe any desirable
* combination of methods. On the other hand, the macros are indices where a
* #server_consts HTTP method literal resides in.
*
* Granted that all method literals in #server_consts are stored in succession,
* it is easy to determine its corresponding #MethodFlag, as follows:
* @verbatim
MethodFlag_bit = 1 << (METHOD_* - METHOD_MIN)@endverbatim
* Where:
*   - @c METHOD_* is a method macro (or, in other word, a #server_consts index),
*       except for #METHOD_MIN.
*   - @c METHOD_MIN is the macro for the index of the first method literal.
*   - @c MethodFlag_bit the #MethodFlag value, METHOD_* corresponds to.
*/
typedef enum {
    HTTP_GET        =  1<<(METHOD_GET - METHOD_MIN),
    HTTP_PUT        =  1<<(METHOD_PUT - METHOD_MIN)
} MethodFlag;

/**
* @brief Responsible for dealing with interrupts at socket 0.
*
* Eventually, it will forward incoming data to HTTP server.
*/
void socket0_handler(uint8_t status);

/**
* @brief Callback function for a particular absolute path.
*
* Provides a callback for a particular (server defined) absolute path. The
* callback function receives a pointer to the #HTTPRequest representation of the
* request and may work with #s_next() and #c_next() to consume the input stream
* and produce an appropriate response. It is safe to assume the callback is only
* executed if the requested method (ie, #HTTPRequest::method) is applicable to
* this particular path (ie, has a corresponding bit set in @link
* HTTPHandler::methods methods@endlink.
* Note that @link ResourceHandler::methods methods@endlink is the bit-wise OR
* result of #MethodFlag values while #HTTPRequest::method is an *index* number
* of a #server_consts string literal.
*
* All paths supported by the server must be issued exactly one handler. Use
* #srvr_set_resource_handler() to that end. Failing to do so for some paths will
* result in a 500 (Internal Server Error) response being returned to the
* requester entity for those paths.
*/
typedef struct {
    /**
    * @brief Bit-wise-OR'ed #MethodFlag values representing the acceptable
    * methods for this resource. */
    uint8_t methods;

    /** @brief Handler callback for this particular path. */
    void (*fn)(HTTPRequest *);
} ResourceHandler;
