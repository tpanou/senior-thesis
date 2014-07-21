/**
* @file
* @addtogroup http_server HTTP Server
* @{
*/

#ifndef HTTP_SERVER_H_INCL
#define HTTP_SERVER_H_INCL

struct ResourceHandler;

#include <inttypes.h>

#define HOST_NAME_LEN   16

#define HOST_PORT_LEN   6

#ifndef NULL
#define NULL            0
#endif

/**
* @brief Various HTTP server settings.
*
* The settings include the Host name and server port, 
*/
typedef struct {

    /**
    * @brief A list of tokens used in parsing HTTP headers.
    *
    * For a detailed description, see #server_consts.
    */
    uint8_t**           consts;

    /**
    * @brief The name of the server (value of `Host' header and part of absolute
    * URIs).
    *
    * This value is compared against (and must match) the host of the request
    * line of incoming HTTP requests that specify an absolute URI. It is set
    * using srvr_set_host_name() or srvr_set_host_name_ip(). It should be noted
    * that no trailing slash should ever be appended.
    */
    uint8_t             host_name[HOST_NAME_LEN];

    /**
    * @brief The listening port of the server; defaults to 80.
    *
    * This value is compared against (and must match) the port of the request
    * line of incoming HTTP requests that specify one. It is set using
    * srvr_set_port().
    */
    uint8_t             host_port[HOST_PORT_LEN];

    /**
    * @brief Reference to the supported absolute path tokens.
    *
    * The value is provided by the resource.h module during its initialisation
    * by calling srvr_set_resource().
    */
    uint8_t**           rsrc_tokens;

    /**
    * @brief An array of resource handlers one for each token in @c rsrc_tokens.
    *
    * Each handler in this array is invoked when the token in the corresponding
    * position of @c rsrc_tokens is found in the HTTP request URI, granted there
    * is a #MethodFlag bit set for that request method (for details, see
    * #ResourceHandler).
    *
    * The value is provided by the resource.h module during its initialisation
    * by calling srvr_set_resource().
    */
    struct
    ResourceHandler*    rsrc_handlers;

    /**
    * @brief The number of token-handler pairs found in @c rsrc_tokens and @c
    * rsrc_handlers.
    *
    * The value is provided by the resource.h module during its initialisation
    * by calling srvr_set_resource().
    */
    uint8_t             rsrc_len;

} ServerSettings;

/**
* @brief A representation of an HTTP message.
*/
typedef struct HTTPRequest {
    /** @brief Value representing the method of the request. */
    uint8_t method;

    /** @brief Value representing the URI of the request line. */
    uint8_t uri;

    /** @brief The major number of the HTTP version of the message. */
    uint8_t v_major;

    /** @brief The minor number of the HTTP version of the message. */
    uint8_t v_minor;

    /** @brief Value representing the accept media range of the request. */
    int8_t accept;

    /** @brief Value representing the transfer encoding of the message. */
    uint8_t transfer_encoding;

    /** @brief Value representing the content type of the message. */
    uint8_t content_type;

    /** @brief The length (in octets) of the message. */
    uint16_t content_length;
} HTTPRequest;


/**
* @brief General-context macro for any parameter not set to a known value.
*/
#define SRVR_NOT_SET   (0xFF)

/**
* @brief The starting index in #server_consts of supported method literals.
*/
#define METHOD_MIN            0
#define METHOD_CONNECT        0 /**< @brief Method @c CONNECT. */
#define METHOD_DELETE         1 /**< @brief Method @c DELETE. */
#define METHOD_HEAD           2 /**< @brief Method @c HEAD. */
#define METHOD_GET            3 /**< @brief Method @c GET. */
#define METHOD_OPTIONS        4 /**< @brief Method @c OPTIONS. */
#define METHOD_POST           5 /**< @brief Method @c POST. */
#define METHOD_PUT            6 /**< @brief Method @c PUT. */
#define METHOD_TRACE          7 /**< @brief Method @c TRACE. */
/**
* @brief The number of HTTP method tokens.
*/
#define METHOD_MAX            8

/**
* @brief The starting index in #server_consts of supported header literals.
*/
#define HEADER_MIN           (METHOD_MAX)
#define HEADER_ACCEPT         0 /**< @brief Header @c Accept. */
#define HEADER_CONTENT_LENGTH 1 /**< @brief Header @c Content-Length. */
#define HEADER_CONTENT_TYPE   2 /**< @brief Header @c Content-Type. */
#define HEADER_TRANSFER_ENC   3 /**< @brief Header @c Transfer-Encoding. */
/**
* @brief The number of HTTP header tokens.
*/
#define HEADER_MAX            4

/**
* @brief The starting index in #server_consts of supported media range literals.
*/
#define MIME_MIN             (METHOD_MAX+HEADER_MAX)
#define MIME_ANY              0 /**< @brief Media range "* / *". */
#define MIME_APP_ANY          1 /**< @brief Media range "application/any". */
#define MIME_APP_JSON         2 /**< @brief Media range "application/json". */
#define MIME_TEXT_ANY         3 /**< @brief Media range "text/ *". */
#define MIME_TEXT_HTML        4 /**< @brief Media range "text/html". */
#define MIME_TEXT_JSON        5 /**< @brief Media range "text/json". */
/**
* @brief The number of media range literals.
*/
#define MIME_MAX              6

/**
* @brief The starting index in #server_consts of available transfer-codings.
*/
#define TRANSFER_COD_MIN    (METHOD_MAX+HEADER_MAX+MIME_MAX)
#define TRANSFER_COD_CHUNK    0 /**< @brief Chunked transfer-coding. */
#define TRANSFER_COD_IDENT    1 /**< @brief Identity transfer-conding. */
/**
* @brief The number of transfer-coding literals.
*/
#define TRANSFER_COD_MAX     2

/** @brief HTTP literal. */
#define HTTP_SCHEME          (METHOD_MAX+HEADER_MAX+MIME_MAX+TRANSFER_COD_MAX)

/** @brief HTTP scheme with separator. */
#define HTTP_SCHEME_S        (HTTP_SCHEME + 1)

/**
* @brief Convert a METHOD_ macro to a MethodFlag bit.
*/
#define TO_METHOD_FLAG(x)      (1<<(x - METHOD_MIN))

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
*
* Also, see #TO_METHOD_FLAG.
*/
typedef enum {
    /** @brief Method OPTIONS flag-bit. */
    HTTP_OPTIONS    =  TO_METHOD_FLAG(METHOD_OPTIONS),

    /** @brief Method GET flag-bit. */
    HTTP_GET        =  TO_METHOD_FLAG(METHOD_GET),

    /** @brief Method PUT flag-bit. */
    HTTP_PUT        =  TO_METHOD_FLAG(METHOD_PUT),

    /** @brief Method POST flag-bit. */
    HTTP_POST       =  TO_METHOD_FLAG(METHOD_POST)
} MethodFlag;

/**
* @brief Register the specified resource tokens and handlers with the server.
*
* All paths supported by the server must be issued exactly one handler this way.
* Failing to do so for some paths will result in a 404 (Not Found) response
* being returned to the requester entity for those paths. This function need
* only be invoked once, unless the address of the arrays changes (ie, switching
* individual handlers during run-time -- via rsrc_set_handler() -- does not
* affect the array address).
*
* Passing @c NULL or @c 0 to any of the parameters discards all resource
* references.
*
* @param[in] tokens Supported absolute path tokens.
* @param[in] handlers Array of functions to call for each string in @p tokens.
* @param[in] len The number of token-handler pairs found in @p tokens and @p
* handlers.
*/
void srvr_set_resources(uint8_t** tokens,
                        struct ResourceHandler* handlers,
                        uint8_t len);

/**
* @brief Convert and set an IP address as the host name of the HTTP server.
*
* This function is an alternative to set_host_name(). It receives an array of
* four bytes and converts them to an equivalent IP address string
* (null-terminated).
*
* More specifically, for each address byte, an initial position is estimated and
* then incremented dependent on the number of its digits. Then, a character is
* passed into #host_name for each of its digits (using modulo and quotient).
*
* @param[in] ip A four-byte array of an IP address.
*/
void srvr_set_host_name_ip(uint8_t* ip);

#endif /* HTTP_SERVER_H_INCL */
/** @} */
