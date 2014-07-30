
#include "json_parser.h"
#include "w5100/socket.h"
#include "util.h"
#include "defs.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

/**
* @ingroup json_parser
* @brief Function pointer to access the next character to parse.
*
* It provides this module's components access to the input stream. In a typical
* application, this should be the same stream that supplies characters to
* external helper functions (such as stream_match()). It is set using
* json_set_source().
*/
static int8_t (*gnext)(uint8_t*);

void json_set_source(int8_t (*input_source)(uint8_t*)) {
    gnext = input_source;
}

int8_t json_parse(uint8_t** tokens, ParamValue* values, uint8_t len) {
    int8_t c_type;      /* Operation status (return value). */
    uint8_t c;          /* Passed from one stream parser function to another. */
    uint8_t i;

    /* A wrapper around tokens (strings) and ParamValues used by the underlying
    * API. */
    ParamInfo match = {.tokens = tokens, .values = values, .len = len};

    /* Reset the status bits of the params that are being searched for. */
    for(i = 0 ; i < len ; ++i) {
        values[i].status_len  &= ~PARAM_STATUS_MASK;
    }

    /* Discard leading white-space from the stream. An initial white-space
    * character is faked by setting @c c to space. */
    c = ' ';
    c_type = json_discard_WS(&c);

    if(!c_type) {
        /* Only objects are supported at this time. It the content is not a
        * valid object, #OTHER will be returned. */
        c_type = json_parse_object(&match, &c);
    }

    return c_type;
}

void json_serialise(uint8_t** tokens, ParamValue* values, uint8_t len) {
    uint8_t buf[6]      =  " { \"";     /* A local buffer. */
    uint8_t* str        =  buf;         /* String to send. */
    uint8_t k           =  4;           /* Number of bytes to send. */
    uint8_t i           =  0;           /* Iteration of @p tokens. */
    uint8_t j;                          /* Pad numbers with white-space. */
    uint8_t flush       =  0;           /* Flush data after sending them. */

    uint8_t state       =  JSON_OBJECT_BEGIN;

    while(len) {

        switch(state) {
            case JSON_OBJECT_BEGIN:
                state       =  JSON_KEY_BEGIN;
            break;

            case JSON_KEY_BEGIN:
                /* Print token. A double quote has been previously sent. */
                str         =  tokens[i];
                k           =  strlen(str);
                state       =  JSON_KEY_END;
            break;

            case JSON_KEY_END:

                buf[k++]    =  '"';     /* Key-end. */
                buf[k++]    =  ' ';
                buf[k++]    =  ':';
                buf[k++]    =  ' ';

                /* Insert a double quote if a string is to be printed. */
                if(values[i].type == DTYPE_STRING) {
                    buf[k++]    =  '"';
                }

                str         =  buf;
                state       =  JSON_VALUE_BEGIN;
            break;

            case JSON_VALUE_BEGIN:

                switch(values[i].type) {
                    case DTYPE_UINT:
                        /* Print a total of 5 digits (with padding to fill the
                        * gaps, if needed). */
                        k   =  5;

                        /* The size of padding. */
                        j   =  4 - uint_to_str(&buf[5],
                                             *((uint8_t*)values[i].data_ptr));

                        /* Pad with spaces to create a fixed-width number. */
                        while(j) {
                            buf[j] = ' ';
                            --j;
                        }
                        buf[j] = ' ';

                        str =  buf;

                    break;
                    case DTYPE_STRING:
                        str =  (uint8_t*)values[i].data_ptr;
                        k   =  strlen(str);

                    break;
                }
                state       =  JSON_VALUE_END;
            break;

            case JSON_VALUE_END:

                /* If a string was printed, append a closing double quote. */
                if(values[i].type == DTYPE_STRING) {
                    buf[k++] = '"';
                }

                buf[k++]    =  ' ';

                ++i;        /* Increase number of iteration. */
                --len;      /* Decrease remainder of parameters. */

                /* Append a comma, if more parameters follow, or a brace,
                * otherwise. */
                if(len) {
                    buf[k++]    =  ',';
                    buf[k++]    =  ' ';
                    buf[k++]    =  '"';     /* Key-start. */
                    state = JSON_KEY_BEGIN;
                } else {
                    buf[k++]    =  0x7d; /* } */
                    buf[k++]    =  ' ';
                    flush       =  1;
                }

                str         = buf;
                break;
        }

        send(HTTP_SOCKET, str, k, flush);

        k           =  0;
    }
}

int8_t json_discard_WS(uint8_t* c) {
    int8_t c_type = 0;

    while(!c_type && JSON_IS_WS(*c)) {
        c_type = (*gnext)(c);
    }
    return c_type;
}

static int8_t json_parse_object(ParamInfo* info, uint8_t* c) {
    int8_t state = JSON_OBJECT_BEGIN;
    int8_t go_on = 1;
    int8_t c_type = 0;

    while(!c_type && go_on) {
        c_type = json_discard_WS(c);
        if(c_type == EOF) break;

        switch(state) {
            case JSON_OBJECT_BEGIN:
                if(*c == 0x7b) {    /* { */
                    c_type  = (*gnext)(c);
                    state   = JSON_MEMBER_BEGIN;
                } else {
                    c_type = OTHER;
                }
            break;
            case JSON_MEMBER_BEGIN:
                if(*c == '"') {
                    c_type  = json_parse_member(info, c);
                    state = JSON_MEMBER_END;
                } else if(*c == 0x7d) {     /* } */
                    go_on   = 0;
                    /* state   = JSON_OBJECT_END; */
                } else {
                    c_type = OTHER;
                }
            break;
            case JSON_MEMBER_END:
                if(*c == ',') {
                    c_type  = (*gnext)(c);
                    state   = JSON_MEMBER_BEGIN;
                } else if(*c == 0x7d) {
                    go_on   = 0;
                    /* state   = JSON_OBJECT_END; */
                } else {
                    c_type = OTHER;
                }
            break;
        }
    }
    return c_type;
}

static int8_t json_parse_member(ParamInfo* info, uint8_t* c) {
    int8_t c_type;      /* Return value of various functions (incl this one). */
    int8_t state;       /* The current state of processing. */
    int8_t match;       /* Index of a matching token. */
    int8_t go_on = 1;   /* @c 0 indicates member parsing has been completed. */

    if(*c == '"') {
        c_type = (*gnext)(c);
        state  = JSON_KEY_BEGIN;
    } else {
        c_type = OTHER;
    }

    while(!c_type && go_on) {
        c_type = json_discard_WS(c);
        if(c_type == EOF) break;

        switch(state) {
            case JSON_KEY_BEGIN:
                c_type = stream_match(info->tokens, info->len, c);

                /* If #stream_match() found a candidate token (c_type > 0) with
                * a proper terminator ('"', in this case), then an acceptable
                * key has been detected. Discard white-space, colon and any
                * further white-space before calling the value-parser. */
                if(*c == '"') {
                    match   = c_type;
                    c_type  = (*gnext)(c);
                    state   = JSON_KEY_END;
                } else {
                    c_type = OTHER;
                }
            break;
            case JSON_KEY_END:
                if(*c == ':') {
                    c_type  = (*gnext)(c);
                    state   = JSON_VALUE_BEGIN;
                } else {
                    c_type = OTHER;
                }
            break;
            case JSON_VALUE_BEGIN:
                c_type  = json_parse_value(&info->values[match], c);
                go_on   = 0;
                /* state   = JSON_VALUE_END; */
            break;
        }
    }
    return c_type;
}

static int8_t json_parse_value(ParamValue* pvalue, uint8_t* c) {
    int8_t c_type;

    /* Acceptable length (if string) or resolution (if uint) of value. */
    uint8_t size            =  pvalue->status_len & 0x3F;

    /* Status to apply to this param; default is  to assume it is valid. */
    uint8_t status          =  PARAM_VALID;

    switch(pvalue->type) {
        case DTYPE_UINT:

            if(size == 8) {
                c_type  = parse_uint8((uint8_t*)(pvalue->data_ptr), c);
            } else if(size == 16) {
/*                c_type  = parse_uint16((uint16_t*)(pvalue->data_ptr), c);*/
            }

            /* `parse_uint' returns #OTHER when there are more digits available
            * than the supported resolution. */
            if(c_type == OTHER) {
                status  =  PARAM_TOO_LONG;

            /* Check whether termination occurred due to invalid character. */
            } else if(!JSON_IS_WS(*c) && *c != ',' && *c != 0x7d /* } */ ) {
                status  =  PARAM_INVALID;
                c_type  =  OTHER;
            }
        break;

        case DTYPE_STRING:
            /* In JSON, strings begin with a '"'. If there is not one, then it
            * is not a valid string. */
            if(*c != '"') {
                status  =  PARAM_INVALID;
                c_type  =  OTHER;
                break;
            }

            /* Read the first character after '"' and start copying into the
            * assigned buffer. */
            c_type = (*gnext)(c); if(c_type == EOF) break;
            c_type = copy_until((uint8_t*)(pvalue->data_ptr), '"', size, c);

            /* `copy_until' returns #OTHER upon failing to read the delimiter
            * before exceeding its allowed amount of characters. */
            if(c_type == OTHER) {
                status  =  PARAM_TOO_LONG;

            /* Read the character after the terminating '"' to return it to the
            * callee. */
            } else {
                c_type = (*gnext)(c);
            }
        break;
    }

    /* Apply value conformance status to it. Note that the two MSB are used to
    * that end. */
    pvalue->status_len |=  status;
    return c_type;
}
