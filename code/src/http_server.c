#include "http_server.h"
#include "http_parser.h"
#include "json_parser.h"
#include "resource.h"

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
uint8_t* server_consts[] = {
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
    /* TRANFSER_CODINGS, min: METHODS+HEADERS+MEDIA_RANGES, max: 2 */
    "chunked",
    "identity",
    /* HTTP TOKENS, indices: METHODS+HEADERS+MEDIA_RANGES+T_CONDINGS, +1 */
    "http",
    "http://"
};

ServerSettings settings = {
    .consts         =  server_consts,

    .host_name      =  "000.000.000.000",
    .host_port      =  "80",

    .rsrc_tokens    =  NULL,
    .rsrc_handlers  =  NULL,
    .rsrc_len       =  0
};

void srvr_set_resources(uint8_t** tokens,
                        ResourceHandler* handlers,
                        uint8_t len) {

    if(tokens && handlers && len) {
        settings.rsrc_tokens    =  tokens;
        settings.rsrc_handlers  =  handlers;
        settings.rsrc_len       =  len;
    } else {
        settings.rsrc_tokens    =  NULL;
        settings.rsrc_handlers  =  NULL;
        settings.rsrc_len       =  0;
    }
}

void srvr_set_host_name_ip(uint8_t* ip) {
    uint8_t byte;   /* A single byte from @p ip. */
    uint8_t i;      /* For each byte in @p ip. */
    uint8_t pos;    /* Position in ServerSettings#host_name to write to next. */
    uint8_t j;      /* Digit of @c byte to write next. */
    uint8_t* host = settings.host_name;

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
