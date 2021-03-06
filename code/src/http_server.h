/**
* @file
* @addtogroup http_server HTTP Server
* @{
*/

#ifndef HTTP_SERVER_H_INCL
#define HTTP_SERVER_H_INCL

#include "resource.h"

#include <inttypes.h>

/**
* @brief Maximum server name length (including null-byte).
*/
#define HOST_NAME_LEN   16

/**
* @brief Maximum server port length (including null-byte).
*/
#define HOST_PORT_LEN   6

/**
* @brief Size of the longest TXF_* macro.
*
* To conserve SRAM, some text fragments (TXF_* macros) are stored in program
* space (Flash memory) and only loaded into main memory when required. This
* macro defines the size of the automatic (local) buffer that will be populated
* with any requested text fragments and should be large enough to contain the
* longest TXF_* string.
*/
#define TXF_BUF_LEN     44

#ifndef NULL
/**
* Specify that a pointer has not been set to a valid address.
*/
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

    /**
    * @brief Permissible query parameter tokens.
    *
    * Passing an #HTTPRequest variable to rsrc_inform(), *after* .method and
    * .uri have been set, will update
    * @link QueryString#tokens .tokens@endlink and
    * @link QueryString#count .count@endlink of .query to the acceptable options
    * for that Resource and method.
    */
    QueryString         query;
} HTTPRequest;

/**
* @brief The total amount of text fragments that may be used with
* srvr_compile().
*/
#define TXF_MAX              27
#define TXF_SPACE             0 /**< @brief A single space. */
#define TXF_COLON             1 /**< @brief A single colon. */
#define TXF_CRLF              2 /**< @brief A CRLF sequence (0x0D, 0x0A). */
#define TXF_STATUS_200        3 /**< @brief The text: 200 OK */
#define TXF_STATUS_404        4 /**< @brief The text: 404 Not Found */
#define TXF_STATUS_405        5 /**< @brief The text: 405 Method Not Allowed */
#define TXF_STATUS_501        6 /**< @brief The text: 501 Not Implemented */
#define TXF_HTTPv             7 /**< @brief The text: HTTP/1.1 */
#define TXF_ALLOW             8 /**< @brief The text: Allow */
#define TXF_CONNECTION_CLOSE  9 /**< @brief The text: Connection: close */
#define TXF_CONTENT_LENGTH   10 /**< @brief The text: Content-Length */
#define TXF_CONTENT_TYPE     11 /**< @brief The text: Content-Type */
#define TXF_SERVER           12 /**< @brief Header Server and its value. */
#define TXF_COMMA            13 /**< @brief A  single comma. */
#define TXF_RETRY_AFTER      14 /**< @brief The text: Retry-After */
#define TXF_STATUS_202       15 /**< @brief The text: 202 Accepted */
#define TXF_STATUS_400       16 /**< @brief The text: 400 Bad Request */
#define TXF_STATUS_503       17 /**< @brief The text: 503 Service Unavailable */
#define TXF_SEMICOLON        18 /**< @brief A single semicolon. */
#define TXF_CHUNKED          19 /**< @brief Chunked transfer encoding header. */
#define TXF_CHAR_UTF8        20 /**< @brief The text: charset=utf-8 */
#define TXF_JSON_LINE        21 /**< @brief A complete JSON type header line. */
#define TXF_GZIP_LINE        22 /**< @brief A complete gzip encoding line. */
#define TXF_JS_LINE          23 /**< @brief A complete JS type header line. */
#define TXF_CSS_LINE         24 /**< @brief A complete CSS type header line. */
#define TXF_CACHE_NO_CACHE   25 /**< @brief The text: Cache-Control:no-cache */
#define TXF_CACHE_PUBLIC     26 /**< @brief The text: Cache-Control:public */

/**
* @brief Alias of #TXF_SPACE.
*/
#define TXF_SP                  TXF_SPACE

/**
* @brief Alias of #TXF_COLON.
*/
#define TXF_HS                  TXF_COLON

/**
* @brief Alias of #TXF_CRLF
*/
#define TXF_ln                  TXF_CRLF

/**
* @brief Empty line (CRLF,CRLF).
*/
#define TXF_lnln                TXF_CRLF, TXF_CRLF

/**
* @brief `Content-Length' header with a value of @c 0.
*/
#define TXF_CONTENT_LENGTH_ZERO_ln \
TXF_CONTENT_LENGTH, TXF_HS, TXFx_FW_UINT, 0, TXF_ln

/**
* @brief A set a headers that should be present in all responses.
*/
#define TXF_STANDARD_HEADERS_ln \
TXF_SERVER, TXF_CRLF,           \
TXF_CONNECTION_CLOSE, TXF_CRLF

/**
* @brief Equivalent to #srvr_compile(1, ..., #SRVR_NOT_SET).
*
* Convenience macro to avoid specifying #SRVR_NOT_SET as the last argument and
* @c 1 as the first.
*/
#define srvr_send(...) \
srvr_compile(1, __VA_ARGS__, SRVR_NOT_SET)

/**
* @brief Equivalent to #srvr_compile(0, ..., #SRVR_NOT_SET).
*
* Convenience macro to avoid specifying #SRVR_NOT_SET as the last argument and
* @c 0 as the first.
*/
#define srvr_prep(...) \
srvr_compile(0, __VA_ARGS__, SRVR_NOT_SET)

/**
* @brief Pass any custom string into srvr_compile().
*
* Causes the next argument to be interpreted as a (null-terminated) string which
* is copied into the output buffer as-is.
*/
#define TXFx_FW_STRING      254

/**
* @brief Pass an unsigned number into srvr_compile().
*
* Causes the next argument to be converted into a string and then copied into
* the output buffer.
*/
#define TXFx_FW_UINT        253

/**
* @brief Make the next string printed by srvr_compile() appear in upper-case.
*
* Causes the next text fragment to be converted into upper-case before copying
* it into the output buffer. Note that if a custom string was supplied (with
* #TXFx_TO_ALLCAP), the original string will be altered.
*/
#define TXFx_TO_ALLCAP      252

/**
* @brief Use a text fragment that resides in main memory in srvr_compile().
*
* Causes the next argument to be interpreted as a constant from #server_consts.
* Currently, they have to be explicitly declared 
*/
#define TXFx_FROMRAM        251

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
#define METHOD_GET            2 /**< @brief Method @c GET. */
#define METHOD_HEAD           3 /**< @brief Method @c HEAD. */
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

/**
* @brief Describes an unsupported transfer-coding value or combination thereof.
*
* This is not a #server_consts index.
*/
#define TRANSFER_COD_OTHER    TRANSFER_COD_MAX

/** @brief HTTP literal. */
#define HTTP_SCHEME          (METHOD_MAX+HEADER_MAX+MIME_MAX+TRANSFER_COD_MAX)

/** @brief HTTP scheme with separator. */
#define HTTP_SCHEME_S        (HTTP_SCHEME + 1)

/**
* @brief Convert a METHOD_ macro to a MethodFlag bit.
*/
#define TO_METHOD_FLAG(x)      (1<<(x - METHOD_MIN))

/**
* @brief Convenience macro to print: Content-Type:application/json;charset=utf-8
*/
#define TXF_CONTENT_TYPE_JSON_ln    TXF_JSON_LINE, TXF_ln

/**
* @brief Convenience macro to print: Content-Encoding:gzip
*/
#define TXF_GZIP_ln                 TXF_GZIP_LINE, TXF_ln

/**
* @brief Convenience macro to print: Content-Type:type/javascript;charset=utf-8
*/
#define TXF_CONTENT_TYPE_JS_ln      TXF_CONTENT_TYPE,   TXF_HS, \
                                    TXF_JS_LINE,        TXF_ln

/**
* @brief Convenience macro to print: Content-Type:text/css
*/
#define TXF_CONTENT_TYPE_CSS_ln     TXF_CONTENT_TYPE,   TXF_HS, \
                                    TXF_CSS_LINE,       TXF_ln
/**
* @brief Convenience macro to print: Cache-Control:no-cache
*/
#define TXF_CACHE_NO_CACHE_ln       TXF_CACHE_NO_CACHE, TXF_ln

/**
* @brief Convenience macro to print: Cache-Control:public
*/
#define TXF_CACHE_PUBLIC_ln         TXF_CACHE_PUBLIC,   TXF_ln

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
* @brief Initialise HTTP server modules.
*
* It is enough to call this only once.
*/
void srvr_init();

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
* This function is an alternative to set_host_name(). It uses inet_to_str() for
* the conversion.
*
* @param[in] ip A four-byte array of an IP address.
*/
void srvr_set_host_name_ip(uint8_t* ip);

/**
* @brief Send the chunk size of a chunk.
*
* Insignificant leading zeros are not prohibited by the specification
* (<a href="http://tools.ietf.org/html/rfc2616#section-3.6.1">RFC2616 -
* 3.6.1 Chunked Transfer Coding</a>) and, so, four hexadecimal digits are always
* printed, followed by a @c CRLF sequence. The output is not flushed.
*
* @param[in] len The size of the chunk. This will be converted into hexadecimal
*   notation.
* @returns The outcome of send() (for details, see the return value of
*   srvr_compile()).
*/
int16_t srvr_prep_chunk_head(uint16_t num);

/**
* @brief Compiles a response based on the specified text fragments.
*
* The header section of all HTTP responses should be compiled from pieces of
* text taken from a common pool of fragments and put together in the appropriate
* order. This is done through this function to avoid re-definition of similar
* fragments and blocks of code, as well as to hide any internal caching
* mechanisms applied on them.
*
* Internally, this function uses send() to communicate the corresponding text
* fragments to the network module which, in turn, preserves them until send() is
* called with its @c flush argument set to a non-zero value. This could be done
* through this function when @p flush holds a non-zero value or directly with
* send(). Additional data may be sent directly to the network module (with
* send()) at any time, regardless whether this function has previously been or
* will be called again.
*
* As stated in the prototype, this function receives any amount of optional
* arguments which correspond to the text fragments to write to the network
* module. For a list of available fragments, see the TXF_* macros specified
* herein. Obviously, the order in which the macros are specified is important
* because it affects the order in which the corresponding text fragments are
* written to the network module. The arbitrary list of optional arguments is
* terminated by passing #SRVR_NOT_SET as the *last argument*. Failing to do so
* involves the risk of stack overflow and arbitrary text fragments sent to the
* network module.
*
* @param[in] flush Designates whether send() should be called with the intention
*   to return its buffered data to the requester entity, after the specified
*   fragments of this function have been sent to it.
* @param[in] ... Any series of TXF_* macros that specify which text fragments
*   and in what order should be sent to the network module. The last argument
*   *should always* be #SRVR_NOT_SET.
* @returns The outcome of send(): a non-negative number is the amount of
*   available bytes in the networks module's output buffer after appending the
*   specified fragments; a negative number indicates that appending the
*   specified fragments has stopped because one of them could not fit in the
*   network module's currently available buffer space due to lack of as many
*   bytes as the returned value.
*/
int srvr_compile(uint8_t flush, ...);

/**
* @brief Notify data have arrived on the HTTP server's socket.
*
* This function is responsible for performing all necessary initialisation,
* parsing the incoming data as an HTTP request and returning an appropriate
* response to the requester entity, either via the use of a user-defined handler
* (see, #ResourceHandler) or one of predefined messages in case of an exception.
*/
void srvr_call();

#endif /* HTTP_SERVER_H_INCL */
/** @} */
