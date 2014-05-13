
#include "webserver.h"
#include "../build/util.h"

static int8_t parse_header_param_qvalue(uint16_t* qvalue, uint8_t* c) {
    int8_t c_type;

    c_type = s_next(c);

    if(c_type != EOF) {
        c_type = discard_LWS(c); /* Ignore any linear white space. */

        /* Check whether the function above was terminated because a q-value
        * parameter has occurred. */
        if(*c == 'q') {
            *c = ' ';
            c_type = discard_LWS(c);

            if(*c == '=') {
                *c = ' ';
                c_type = discard_LWS(c);

                /* Try to convert q-value from stream. If it is not a valid
                * value, call discard_param() on the rest of the input. */
                if(c_type == OTHER) {
                    q_value(qvalue, c);
                }
            }
        }

        if(*c != ';' && *c != ',') {

            /* Discard the (rest of the) parameter. */
            c_type = discard_param(c);
        }
    }
    return c_type;
}

int8_t q_value(uint16_t* value, uint8_t* c) {
    uint8_t i;
    int8_t  c_type = OTHER;

    *value = 0;

    if(*c == '0') {
        c_type = s_next(c);

        if(*c == '.') {
            /* A maximum of three digits are read; the rest are dropped. */
            c_type = s_next(c);
            for(i = 0 ; i < 3 && *c >= '0' && *c <= '9' ; ++i) {
                *value  *= 10;
                *value  += *c - 0x30;
                c_type = s_next(c);
            }

            /* Imitate 3 digits had been given. */
            for(i ; i < 3 ; ++i) {
                *value  *= 10;
            }
        }
    } else {
        *value = 1000;
    }
    return c_type;
}

static int8_t discard_LWS(uint8_t* c) {
    uint8_t peek;

    do {
        if(*c == '\r') {
            if(s_peek(&peek, 0)) return EOF;

            if(peek == '\n') {
                if(s_peek(&peek, 1)) return EOF;

                /* A CR followed by a LN and a LWSP-char is a valid LWS. */
                if(peek == ' ' || peek == '\t') {
                    /* Discard LF and LWSP-char from the stream. */
                    s_drop(2);
                    *c = peek;
                    continue;
                }
                *c = '\n';
                s_drop(1); /* Discard LF from stream. */
                return CRLF;
            }
            return OTHER;

       /* A single LWSP-char is a valid LWS, anything else is not. */
        } else if (*c != ' ' && *c != '\t') {
            return OTHER;
        }
    } while(!s_next(c));

    return EOF;
}

static int8_t discard_param(uint8_t* c) {
    /* Identifies whether a quoted-string is currently traversed. */
    uint8_t is_quoted_string = 0;

    /* The return status of this function. Generally, it describes the type of
    * the last character read. */
    int8_t c_type = 0;
    uint8_t peek;

    /* While none of CRLF, EOF or OTHER has been found, proceed with the next
    * character. */
    do {
        /* It should be noted that the stream may contain double quotes,
        * in which case they should be matched to an appropriate matching quote
        * before identifying a ";" or a ",". Also, within quoted-strings, a "\"
        * creates a quoted-pair, effectively escaping the following character.
        * For example, if that character is a double-quote, it should not
        * terminate the quoted-string. */

        /* Although a "\" should be able to escape a CR, it is not permitted to
        * do so, if that would cancel a CRLF sequence or folding. */

        if(*c == '\\' && is_quoted_string) {
            /* The following code block is equivalent to:
            *   if(!is_CRLF(*c) || !is_LWS(*c) && *c != ' ' && *c != '\t') {
            *       s_drop(1);
            *   }
            */
            c_type = s_peek(&peek, 0);

            if(peek != '\r') {
                *c = peek;
                s_drop(1);
            } else {
                c_type = s_peek(&peek, 1);

                if(*c != '\n') {
                    *c = peek;
                    s_drop(1);
                }
            }

        /* A quoted-string is initiated and terminated by a double quote. */
        } else if(*c == '"') {
            is_quoted_string = !is_quoted_string;

        } else if(*c == '\r') {
            /* Identify CRLF and header folding. */
            c_type = s_peek(&peek, 0);

            if(peek == '\n') {
                c_type = s_peek(&peek, 1);

                if(peek == ' ' || peek == '\t') {
                    *c = peek; /* Update @c c with the character read. */
                    s_drop(2); /* Discard folding from the stream. */
                } else {
                    *c = '\n';
                    c_type = CRLF;
                    s_drop(1);
                    break;
                }
            }
        } else if(!is_quoted_string && (*c == ';' || *c == ',')) {
            c_type = OTHER;
            break;
        }
    } while((c_type = s_next(c)) == 0);

    return c_type;
}

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
