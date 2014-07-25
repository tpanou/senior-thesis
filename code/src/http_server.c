#include "http_server.h"
#include "http_parser.h"
#include "json_parser.h"
#include "resource.h"
#include "sbuffer.h"

#include "w5100/socket.h"

#include <avr/pgmspace.h>
#include <stdarg.h>

uint8_t txf_space[] PROGMEM         = " ";
uint8_t txf_colon[] PROGMEM         = ":";
uint8_t txf_CRLF[] PROGMEM          = "\r\n";
uint8_t txf_status_200[] PROGMEM    = "200 OK";
uint8_t txf_status_404[] PROGMEM    = "404 Not Found";
uint8_t txf_status_405[] PROGMEM    = "405 Method Not Allowed";
uint8_t txf_status_501[] PROGMEM    = "501 Not Implemented";
uint8_t txf_HTTPv[] PROGMEM         = "HTTP/1.1";
uint8_t txf_allow[] PROGMEM         = "Allow";
uint8_t txf_connection_close[] PROGMEM
                                    = "Connection: close";
uint8_t txf_content_length[] PROGMEM
                                    = "Content-Length";
uint8_t txf_content_type[] PROGMEM  = "Content-Type";
uint8_t txf_server[] PROGMEM        = "Server: uServer (TEIA)";

/**
* @ingroup http_server
* @brief Text fragments stored in program space (Flash memory).
*
* In order to conserve main memory (SRAM), some text fragments are store into
* the more abound program memory and loaded into main memory, as needed.
*/
PGM_P srvr_txf[] PROGMEM = {
    txf_space,
    txf_colon,
    txf_CRLF,
    txf_status_200,
    txf_status_404,
    txf_status_405,
    txf_status_501,
    txf_HTTPv,
    txf_allow,
    txf_connection_close,
    txf_content_length,
    txf_content_type,
    txf_server
};

/**
* @ingroup http_server
* @brief Array of server strings.
*
* They can be used to parse incoming HTTP messages (specifically, their header
* section) and copied to various section of the response. Values include HTTP
* methods, header names and header values. A number of convenience macros are
* also available to facilitate access.
*
* The tokens belong to different groups (eg, method tokens, header tokens) which
* are provided in succession within the array. Within each group, the tokens are
* provided in alphabetic order. The groups are separated based on indices, as
* follows:
*   - Methods start at #METHOD_MIN, containing #METHOD_MAX tokens.
*   - Headers start at #HEADER_MIN, containing #HEADER_MAX tokens.
*   - Media ranges start at #MIME_MIN, containing #MIME_MAX tokens.
*   - Transfer codings start at #TRANSFER_COD_MIN, containing #TRANSFER_COD_MAX
*       tokens.
*
* For individual elements, refer to macros starting with the group in question
* (for instance, for methods, check macros starting with "METHOD_").
*
* There are also some additional tokens, "http" and "http://" represented by
* #HTTP_SCHEME and #HTTP_SCHEME_S.
*
* Should the need to provide additional tokens arise, all that is required is
* to place those strings in their respective group, making certain to preserve
* the order and update the corresponding "_MAX" macro to reflect the new number
* of tokens within each affected group. Note, that any new token should be given
* in the same letter case as the rest of the tokens.
*/
static uint8_t* server_consts[] = {
    /* METHODS, min: 0 max: 8 */
    "connect",
    "delete",
    "get",
    "head",
    "options",
    "post",
    "put",
    "trace",
    /* HEADERS, min: METHODS, max: 4 */
    "accept",
    "content-length",
    "content-type",
    "transfer-encoding",
    /* MEDIA RANGES, min: METHODS+HEADERS, max: 6 */
    "*/*",
    "application/*",
    "application/json",
    "text/*",
    "text/html",
    "text/json",
    /* TRANFSER_CODING, min: METHODS+HEADERS+MEDIA_RANGES, max: 2 */
    "chunked",
    "identity",
    /* HTTP TOKENS, indices: METHODS+HEADERS+MEDIA_RANGES+T_CODING, +1 */
    "http",
    "http://"
};

static ServerSettings srvr = {
    .consts         =  server_consts,

    .host_name      =  "000.000.000.000",
    .host_port      =  "80",

    .rsrc_tokens    =  NULL,
    .rsrc_handlers  =  NULL,
    .rsrc_len       =  0
};

void srvr_init() {
    /* Provide a reference of server settings to the HTTP parser. */
    http_parser_set_server(&srvr);

    /* Specify which parser should be used by the resource handlers. Generally,
    * this should be done every time before calling a resource handler. But, in
    * the current implementation, only JSON formatted data are supported. */
    rsrc_set_parser(&json_parse);
}

void srvr_set_resources(uint8_t** tokens,
                        ResourceHandler* handlers,
                        uint8_t len) {

    if(tokens && handlers && len) {
        srvr.rsrc_tokens    =  tokens;
        srvr.rsrc_handlers  =  handlers;
        srvr.rsrc_len       =  len;
    } else {
        srvr.rsrc_tokens    =  NULL;
        srvr.rsrc_handlers  =  NULL;
        srvr.rsrc_len       =  0;
    }
}

void srvr_set_host_name_ip(uint8_t* ip) {
    uint8_t byte;   /* A single byte from @p ip. */
    uint8_t i;      /* For each byte in @p ip. */
    uint8_t pos;    /* Position in ServerSettings#host_name to write to next. */
    uint8_t j;      /* Digit of @c byte to write next. */
    uint8_t* host = srvr.host_name;

    for(i = 0, pos = 0 ; i < 4 ; ++i) {
        byte = ip[i];
        j = 0;
        if(byte >= 10 && byte < 100) ++pos;
        else if(byte >= 100) pos += 2;

        /* Ensure this runs at least one so that a single zero may not be
        * omitted. */
        do {
            host[pos - j] = byte % 10 + '0';
            ++j;
            byte /= 10;
        } while(byte);

        /* Increment for next '.' or terminating null-byte. */
        ++pos;
        /* If more bytes are to follow, place a '.' and further increase @c pos
        * to point at the position to write the next digit to. */
        if(i != 3) host[pos++] = '.';
    }
    host[pos] = '\0';
}

int16_t srvr_compile(uint8_t flush, ...) {
    int16_t outcome = 0;        /* As returned from send(). */
    uint8_t buf[TXF_BUF_LEN];   /* Stores a fragment until it is sent. */
    unsigned int txf_id;        /* Value of any optional argument; txf index. */
    va_list ap;                 /* Optional argument reference. */

    va_start(ap, flush);
    txf_id = va_arg(ap, unsigned int);

    while(txf_id != SRVR_NOT_SET && outcome >= 0) {

        if(txf_id < TXF_MAX) {
            strcpy_P(buf, (PGM_P)pgm_read_word(&srvr_txf[txf_id]));
            outcome = send(0, buf, strlen(buf), 0);
        }

        txf_id = va_arg(ap, unsigned int);
    }

    /* Flush all buffered data, if so specified. This should be avoided in case
    * any of the mentioned text fragments could not be written, because, then,
    * the text would not be complete / correct. */
    if(flush && outcome >= 0) outcome = send(0, buf, 0, 1);

    va_end(ap);
    return outcome;
}

void srvr_call() {
    uint8_t uri;
    HTTPRequest req;

    /* Always reset to s_next() as it may have been altered due to a different
    * transfer coding of the previous request. */
    stream_set_source(&s_next);
    json_set_source(&s_next);

    req     =  http_parse_request();
    uri     =  req.uri;

    /* TODO: If multiple content types of incoming messages are to be supported,
    * the appropriate parser for each request should be set here, *before*
    * calling the resource handler, below. Currently, only JSON is supported and
    * its parser is set only once, during initialisation. See srvr_init() and
    * its line with rsrc_set_parser(). */

    /* If the URI is not available or if no hander is specified, return 404 (Not
    * Found). */
    if(uri == SRVR_NOT_SET || !srvr.rsrc_handlers[uri].call) {
        /* TODO: Return status code. */
        puts(" >> 404 <<");
        return;
    }

    /* Method not recognised by the server. Return 501 (Not Implemented). */
    if(req.method == SRVR_NOT_SET) {
        /* TODO: Return status code. Maybe, provide the supported methods? */
        puts(" >> 501 <<");
        return;
    }

    /* Call the handler, if the requested method has a bit-flag set. */
    if(TO_METHOD_FLAG(req.method) & srvr.rsrc_handlers[uri].methods) {
        puts("Calling resource handler");

        /* Set-up reading a chunked message, if it was so specified in the
        * header. */
        if(req.transfer_encoding == TRANSFER_COD_CHUNK) {
            stream_set_source(&c_next);
            json_set_source(&c_next);
        }

        /* Call the appropriate handler. */
        (*(srvr.rsrc_handlers[uri].call))(&req);

    /* Otherwise, return a 405 (Method Not Allowed), along with an`Allow'
    * header. */
    } else {
        /* TODO: Return status code. */
        puts(" >> 405 <<");
        return;
    }
}
