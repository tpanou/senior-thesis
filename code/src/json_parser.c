
#include "json_parser.h"
#include <stdio.h>
#include <inttypes.h>

static int8_t json_parse_object(ParamInfo* info, uint8_t* c) {
    int8_t stage = JSON_OBJECT_BEGIN;
    int8_t go_on = 1;
    int8_t c_type = 0;

    while(!c_type && go_on) {
        c_type = json_discard_WS(c);
        if(c_type == EOF) break;

        switch(stage) {
            case JSON_OBJECT_BEGIN:
                if(*c == 0x7b) {    /* { */
                    c_type  = (*nchar)(c);
                    stage   = JSON_MEMBER_BEGIN;
                } else {
                    c_type = OTHER;
                }
            break;
            case JSON_MEMBER_BEGIN:
                if(*c == '"') {
                    c_type  = json_parse_member(info, c);
                    stage = JSON_MEMBER_END;
                } else if(*c == 0x7d) {     /* } */
                    go_on   = 0;
                    /* stage   = JSON_OBJECT_END; */
                } else {
                    c_type = OTHER;
                }
            break;
            case JSON_MEMBER_END:
                if(*c == ',') {
                    c_type  = (*nchar)(c);
                    stage   = JSON_MEMBER_BEGIN;
                } else if(*c == 0x7d) {
                    go_on   = 0;
                    /* stage   = JSON_OBJECT_END; */
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
    int8_t stage;       /* The current stage of processing. */
    int8_t match;       /* Index of a matching token. */
    int8_t go_on = 1;   /* @c 0 indicates member parsing has been completed. */

    if(*c == '"') {
        c_type = (*nchar)(c);
        stage  = JSON_KEY_BEGIN;
    } else {
        c_type = OTHER;
    }

    while(!c_type && go_on) {
        c_type = json_discard_WS(c);
        if(c_type == EOF) break;

        switch(stage) {
            case JSON_KEY_BEGIN:
                c_type = stream_match(info->tokens, 0, info->len, c);

                /* If #stream_match() found a candidate token (c_type > 0) with
                * a proper terminator ('"', in this case), then an acceptable
                * key has been detected. Discard white-space, colon and any
                * further white-space before calling the value-parser. */
                if(*c == '"') {
                    match   = c_type;
                    c_type  = (*nchar)(c);
                    stage   = JSON_KEY_END;
                } else {
                    c_type = OTHER;
                }
            break;
            case JSON_KEY_END:
                if(*c == ':') {
                    c_type  = (*nchar)(c);
                    stage   = JSON_VALUE_BEGIN;
                } else {
                    c_type = OTHER;
                }
            break;
            case JSON_VALUE_BEGIN:
                c_type  = json_parse_value(&info->values[match], c);
                go_on   = 0;
                /* stage   = JSON_VALUE_END; */
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
            if(*c == '"') {
                status  =  PARAM_INVALID;
                c_type  =  OTHER;
                break;
            }

            /* Read the first character after '"' and start copying into the
            * assigned buffer. */
            c_type = (*nchar)(c); if(c_type == EOF) break;
            c_type = copy_until((uint8_t*)(pvalue->data_ptr), '"', size, c);

            /* `copy_until' returns #OTHER upon failing to read the delimiter
            * before exceeding its allowed amount of characters. */
            if(c_type == OTHER) {
                status  =  PARAM_TOO_LONG;

            /* Read the character after the terminating '"' to return it to the
            * callee. */
            } else {
                c_type = (*nchar)(c);
            }
        break;
    }

    /* Apply value conformance status to it. Note that the two MSB are used to
    * that end. */
    pvalue->status_len |=  status;
    return c_type;
}

int8_t json_discard_WS(uint8_t* c) {
    int8_t c_type = 0;

    while(!c_type && JSON_IS_WS(*c)) {
        c_type = (*nchar)(c);
    }
    return c_type;
}
