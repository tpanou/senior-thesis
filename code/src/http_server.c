#include "http_server.h"
#include "http_parser.h"
#include "json_parser.h"
#include "resource.h"

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
uint8_t txf_allow[] PROGMEM         = "Allow";

/**
* @ingroup http_server
* @brief Text fragments stored in program space (Flash memory).
*
* In order to conserve main memory (SRAM), some text fragments are store into
* the more abound program memory and loaded into main memory, as needed.
*/
PGM_P srvr_text[] PROGMEM = {
    txf_space,
    txf_colon,
    txf_CRLF,
    txf_status_200,
    txf_status_404,
    txf_status_405,
    txf_status_501,
    txf_allow
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
    int16_t  outcome    =  0;   /* As returned from send(). */
    uint16_t text_id;           /* Value of any optional argument. */
    uint8_t  buf[TXF_MAX];      /* Stores a fragment until it is sent. */
    uint8_t  size;              /* Content size in @c buf. */
    va_list  ap;                /* Reference to optional argument. */

    va_start(ap, flush);

    /* Send fragments to the network module until #SRVR_NOT_SET is met or the
    * network's module buffer is depleted. */
    while(text_id != SRVR_NOT_SET && outcome >= 0) {
        text_id = va_arg(ap, unsigned int);

        if(text_id < TXF_MAX) {
            strcpy_P(buf, (PGM_P)pgm_read_word(&srvr_text[text_id]));
            outcome = send(0, buf, strlen(buf), 0);

        }
    }

    /* Flush all buffered data, if so specified. This should be avoided in case
    * any of the mentioned text fragments could not be written, because, then,
    * the text would not be complete / correct. */
    if(flush && outcome >= 0) outcome = send(0, buf, 0, flush);

    va_end(ap);
    return outcome;
}
