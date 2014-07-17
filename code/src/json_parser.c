
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
