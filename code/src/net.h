/**
* @file
*/

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
* this particular path.
*
* All paths supported by the server must be issued exactly one handler. Use
* #set_resource_handler() to that end. Failing to do so for some paths will
* result in a 500 (Internal Server Error) being returned to the requester entity
* for those paths.
*/
typedef struct {
    /** @brief Value representing the URI of the request line. */
    uint8_t path;

    /** @brief Handler callback for the specified #ResourceHandler::path. */
    void (*fn)(HTTPRequest *);
} ResourceHandler;
