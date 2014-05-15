
#include "web_server.h"
#include "../build/util.h"

#include <ctype.h> /* isxdigit(), tolower() */

int8_t parse_http_version(HTTP_Message* req, uint8_t* c) {
    int8_t c_type;
    uint8_t digits;

    c_type = stream_match(server_consts, HTTP_SCHEME, HTTP_SCHEME + 1, c);

    if(c_type >= 0 && *c == '/') {

        s_next(c);
        c_type = parse_uint8(&(req->v_major), c);

        while(*c != '.' && c_type != EOF && !is_CRLF(*c)) {
            c_type = s_next(c);
        }

        if(*c == '.') {
            c_type = s_next(c);
            c_type = parse_uint8(&(req->v_minor), c);
        }
    }
    return c_type;
}

int8_t parse_headers(HTTP_Message* req, uint8_t* c) {
    uint16_t qvalue     = 0;
    uint8_t is_emptyln  = 0;
    int8_t c_type       = 0;
    int8_t idx;
    uint8_t peek;

    while(c_type != EOF && !is_emptyln) {
        /* Attempt to identify a header */
        c_type = stream_match(server_consts, HEADER_MIN, HEADER_MAX, c);

        /* If there is a potential match terminated by a ":", then that match is
        * valid. (No sort spacing is allowed in the header-name.) */
        if(c_type >= 0 && *c == ':') {
            idx     = c_type;
            *c       = ' ';  /* Discarding starts from the provided byte. */
            c_type  = discard_LWS(c); /* Ignore LWS on stream. */

            /* Read header-body, if the header-name is not followed by EOF or
            * a CRLF sequence. */
            if(c_type == OTHER) {
                if(idx == HEADER_ACCEPT) {
                    c_type = parse_header_accept(&(req->accept), &qvalue, c);
                }
            }

        /* In case of an unsupported header, the line should be discarded. */
        } else {
            do {
                if(*c == '\r' && is_CRLF(*c)) {
                    c_type = CRLF;
                    s_next(c); /* Load LF into @c c. */
                    break;
                }
            } while((c_type = s_next(c)) != EOF);
        }

        /* Check whether the end of headers has been reached. */
        if(c_type == CRLF) {
            s_peek(&peek, 0);

            if(peek == '\r') {
                s_peek(&peek, 1);

                if(peek == '\n') {
                    s_drop(2);  /* Discard the second CRLF sequence. */
                    is_emptyln = 1;
                }

            /* There exists a new header; read its first byte. */
            } else {
                c_type = s_next(c);
            }
        }
    }
    return c_type;
}

int parse_header_accept(int8_t* media_range, uint16_t* qvalue, uint8_t* c) {
    int8_t c_type;  /* The character type that is read last (eg. EOF, CRLF). */
    int8_t idx;     /* The potentially matched media range. */

    uint16_t qvalue_new = 0;

    do {
        qvalue_new = 1000;  /* Reset q-value to 'preferred'. */

        if(*c == ',') {
            c_type = s_next(c);
            c_type = discard_LWS(c);
            if(c_type == CRLF) break;
        }

        /* Identify the first media range. */
        idx = stream_match(server_consts, MIME_MIN, MIME_MAX, c);
        c_type = discard_LWS(c);

        /* In case that a potential match was terminated by a valid delimiter,
        * try to identify its parameters, and in particular, a q-value. */
        if(idx >= 0) {

            /* An acceptable media range without parameters was specified. In
            * case of an ",", more media ranges may follow. */
            if(c_type == CRLF || *c == ',') {
                *qvalue         = 1000;
                *media_range    = idx;

            } else if(*c == ';') {

                /* Attempt to identify q-value. The last is preserved. */
                do {
                    c_type = parse_header_param_qvalue(&qvalue_new, c);
                } while(*c == ';');

                /* Update media range if it has a higher q-value than the
                * previous. */
                if(qvalue_new > *qvalue) {
                    *qvalue         = qvalue_new;
                    *media_range    = idx;
                }
            }
        }

        /* If an unsupported media range was supplied (ie, one that completely
        * failed a match -- idx < 0 -- or one, that although produced a match,
        * contained no delimiter), it is with certainty that the rest of the
        * line may be safely discarded. If, along the way, a "," is read,
        * the process of identifying a new media range is repeated. */
        while(c_type != EOF && c_type != CRLF && *c != ',') {
            c_type = discard_param(c);

            /* <discard_param>() returns on either a ";" (parameter separator)
            * or a "," (media range separator). If a supported media range was
            * in effect then all its parameters would have been consumed in a
            * previous loop. So this one could only belong to an unsupported
            * media range and must be discarded. */
            if(*c == ';') s_next(c);
        }

    } while(*c == ',');

    return c_type;
}

int stream_match(uint8_t** desc, uint8_t min, uint8_t max, uint8_t* c) {
    uint8_t abs_min = min; /* The initial value (before moving boundaries). */
    uint8_t cmp_idx = 0;
    uint8_t i;
    int8_t c_type   = 0;

    /* When @c min is equal to @c max, the descriptor at position max-1 is a
    * possible match. What is left is to check whether the last character read
    * corresponds to an appropriate delimiter for this particular operation. */
    while(!c_type && min < max) {

        /* The last byte of the descriptors (null-character) is used as an
        * implicit iteration terminator when all the previous characters have
        * been matched. If a null-character is read from the stream when a
        * comparison with the last character of a matching descriptor is due,
        * the comparison will succeed and a new iteration will take place
        * during which the limits of the descriptor will be breached. */
        if(*c == '\0') *c = 1;

        *c = tolower(*c);

        /* Omit descriptors with a character less than @c c. */
        for(i = min; i < max && desc[i][cmp_idx] < *c; ++i)
            ;
        min     = i;

        /* Determine position of last possible match. */
        for(i; i < max && desc[i][cmp_idx] == *c ; ++i)
            ;
        max     = i;

        /* Avoid reading the next character for stream, if lower and upper limit
        * have converged. */
        if(min < max) {
            ++cmp_idx;
            c_type = s_next(c);
        }
    }

    /* If there's been an error reading from stream, return that error.
    * Currently, @c EOF is the only possible error. */
    if(c_type) return c_type;

    /* @c i equals `abs_min' when the input string alphabetically precedes the
    * first currently available. */
    if(i > abs_min && *c != '\0') {

        if(desc[i - 1][cmp_idx] == '\0') return i - 1;
    }

    return OTHER;
}

int8_t stream_match_ext(uint8_t** desc,
                        uint8_t abs_min,
                        uint8_t* min,
                        uint8_t* max,
                        uint8_t* cmp_idx,
                        uint8_t* c) {
    uint8_t i       = 0;
    int8_t c_type   = 0;
    int8_t have_hit = 1;

    /* The iterations continue as long as there is at least one match. */
    while(!c_type && have_hit) {

        have_hit = 0;
        /* Null-character is used implicitly as a comparison terminator. */
        if(*c == '\0') *c = 1;

        *c = tolower(*c);

        /* Omit descriptors with a character less than @c c. */
        for(i = *min; i < *max && desc[i][*cmp_idx] < *c; ++i)
            ;
        *min    = i;

        /* Determine position of last possible match. */
        for(i; i < *max && desc[i][*cmp_idx] == *c ; ++i) {
            have_hit = 1;
        }

        /* Lower upper-bound only if there had been hits. */
        if(have_hit) *max = i;

        /* Read next character if the current one provides a match. Otherwise,
        * the caller should determine if an alternative character should be used
        * in its place. */
        if(*min < *max && have_hit) {
            ++(*cmp_idx);
            c_type = s_next(c);
        }
    }

    /* If there's been an error reading from stream, return that error. */
    if(c_type) return c_type;

    /* @c i equals `abs_min' when the input string alphabetically precedes the
    * first possible literal. */
    if(i > abs_min) {
        if(desc[i - 1][*cmp_idx] == '\0') return i - 1;
    }

    return OTHER;
}

static int8_t parse_uri(HTTP_Message* req, uint8_t* c) {
    uint8_t min     = URI_MIN;
    uint8_t max     = URI_MAX;
    uint8_t peek;
    uint8_t cmp_idx = 0;
    uint8_t last_it = 255;
    int8_t  c_type  = 0;

    /* If the request URI begins with the scheme (ie, HTTP), then an absolute
    * URI is provided and it should match against the server (host) name. If
    * not, an appropriate status code should be returned. */
    s_peek(&peek, 0);
    if(peek != '/' && peek != '*') {
        if(c_type = parse_host(c)) return OTHER;
    }

    /* Parse absolute path. */
    /* Match stream against available absolute paths taking into consideration
    * possible percent-encoding. */
    while(c_type != EOF && min < max) {
        c_type =
            stream_match_ext(server_consts, URI_MIN, &min, &max, &cmp_idx, c);

        /* If there is a failed match for the second time for a particular
        * iteration (ie, after attempting a percent-decoding, if a percent sign
        * was present), then there can be no match. */
        if(last_it == cmp_idx) {
            break;
        }

        /* On a mismatch due to "%", retry the comparison, unless already
        * retried. */
        if(c_type == OTHER && *c == '%' && last_it != cmp_idx) {
            uint8_t p1, p2;
            uint8_t value;

            last_it = cmp_idx; /* Keep iteration of last percent decoding. */

            /* Decode the next two characters. */
            s_peek(&p1, 0);
            s_peek(&p2, 1);

            if(isxdigit(p1) && isxdigit(p2)) {
                s_drop(2);
                value   = p1 <= '9' ? p1 - '0' : tolower(p1) - 'a' + 10;
                value  *= 16;
                value  += p2 <= '9' ? p2 - '0' : tolower(p2) - 'a' + 10;

                *c = value;
            } else {
                break;
            }

        /* A possible match has been admitted. */
        } else if(c_type >= 0) {
            if(*c == ' ') {
                req->uri = c_type;
            }
            break;

        /* Certain mismatch. */
        } else {
            break;
        }
    }

    return c_type;
}

static int8_t parse_host(uint8_t* c) {
    int8_t c_type;
    uint8_t* ptr[1];

    /* Match scheme "HTTP://" */
    c_type = stream_match(server_consts, HTTP_SCHEME_S, HTTP_SCHEME_S + 1, c);

    /* If scheme is acceptable, parse the host name. */
    if(c_type >= 0) {
        ptr[0] = host_name;
        c_type = stream_match(ptr, 0, 1, c);

        /* Should the host name also be acceptable, parse the port, if one is
        * specified. */
        if(c_type >= 0) {

            /* If a port was provided, ensure it matches the host's. */
            if(*c == ':') {
                c_type = s_next(c);  /* Get character next to colon. */
                ptr[0] = host_port;
                c_type = stream_match(ptr, 0, 1, c);

                if(c_type >= 0) return 0;

                /* An unsupported port was specified; discard it. Odd if this
                * happened but still needs to be dealt with. */
                else {
                    while(isdigit(*c)) c_type = s_next(c);
                }
            } else {
                return 0;
            }
        }
    }
    return c_type;
}

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

int8_t parse_uint16(uint16_t* value, uint8_t* c) {
    int8_t c_type;
    int8_t digits;
    *value  = 0;

    while(*c == '0') c_type = s_next(c); /* Ignore any leading zeros. */

    for(digits = 0 ; c_type != EOF && digits < 4 && isdigit(*c) ; ++digits) {
        *value  *= 10;
        *value  += *c - '0';
        c_type = s_next(c);
    }
    if(isdigit(*c)) {
        c_type = OTHER; /* Inform an additional digit is on stream. */
    }
    return c_type;
}

int8_t parse_uint8(uint8_t* value, uint8_t* c) {
    int8_t c_type;
    int8_t digits;
    *value  = 0;

    while(*c == '0') c_type = s_next(c); /* Ignore any leading zeros. */

    for(digits = 0 ; c_type != EOF && digits < 2 && isdigit(*c) ; ++digits) {
        *value  *= 10;
        *value  += *c - '0';
        c_type = s_next(c);
    }
    if(isdigit(*c)) {
        c_type = OTHER; /* Inform an additional digit is on stream. */
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
