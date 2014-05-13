
#include "webserver.h"
#include "../build/util.h"

int8_t is_LWS(uint8_t c) {

    if(c == ' ' || c == '\t') return 1;

    if(c == '\r') {
        if(s_peek(&c, 0)) return 0;

        if(c == '\n') {
            if(s_peek(&c, 1)) return 0;

            /* A CR followed by a LN and a LWSP-char is a valid LWS. */
            if(c == ' ' || c == '\t') {
                return 1;
            }
        }
    }
    return 0;
}

int8_t is_CRLF(uint8_t c) {

    if(c == '\r') {
        if(s_peek(&c, 0)) return 0;

        if(c == '\n') {
            if(s_peek(&c, 1)) return 0;

            /* A CRLF is rendered a LWS, if it is followed by a LWS-char, ie,
            * SPACE or HTAB. */
            if(c != ' ' && c != '\t') return 1;
        }
    }

    return 0;
}
