#include "stream_util.h"

/**
* @ingroup stream_util
* @brief Function pointer to access the next character to parse.
*
* It provides this module's components access to the input stream. It is set
* using stream_set_source(). Attempting to invoke any of the functions included
* in this module without previously setting it to a known value will, in all
* probability, cause the application to fail.
*/
static int8_t (*gnext)(uint8_t*);

void stream_set_source(int8_t (*next_char)(uint8_t*)) {
    gnext = next_char;
}

int8_t parse_hex16(uint16_t* value, uint8_t* c) {
    int8_t c_type   =  0;
    int8_t digits;
    *value          =  0;

    while(*c == '0') c_type = (*gnext)(c); /* Ignore any leading zeros. */

    for(digits = 0 ; c_type != EOF && digits < 4 && isxdigit(*c) ; ++digits) {

        *value  *= 16;
        *value  += *c <= '9' ? *c - '0' : tolower(*c) - 'a' + 10;
        c_type = (*gnext)(c);
    }

    if(isxdigit(*c)) {
        c_type = OTHER; /* Inform an additional digit is on stream. */
    }
    return c_type;
}

int8_t parse_uint16(uint16_t* value, uint8_t* c) {
    int8_t c_type   =  0;
    int8_t digits;
    *value          =  0;

    while(*c == '0') c_type = (*gnext)(c); /* Ignore any leading zeros. */

    for(digits = 0 ; c_type != EOF && digits < 4 && isdigit(*c) ; ++digits) {
        *value  *= 10;
        *value  += *c - '0';
        c_type = (*gnext)(c);
    }
    if(isdigit(*c)) {
        c_type = OTHER; /* Inform an additional digit is on stream. */
    }
    return c_type;
}

int8_t parse_uint8(uint8_t* value, uint8_t* c) {
    int8_t c_type   =  0;
    int8_t digits;
    *value          =  0;

    while(*c == '0') c_type = (*gnext)(c); /* Ignore any leading zeros. */

    for(digits = 0 ; c_type != EOF && digits < 2 && isdigit(*c) ; ++digits) {
        *value  *= 10;
        *value  += *c - '0';
        c_type = (*gnext)(c);
    }
    if(isdigit(*c)) {
        c_type = OTHER; /* Inform an additional digit is on stream. */
    }
    return c_type;
}

int8_t copy_until(uint8_t* buf, uint8_t delim, uint8_t max, uint8_t* c) {
    int8_t c_type   = 0;
    uint8_t i       = 0;

    while(*c != delim && !c_type) {
        if(i == max - 1) {
            c_type == OTHER;
            break;
        }

        buf[i] = *c;
        c_type = (*gnext)(c);
        ++i;
    }

    /* Append a string delimiter. */
    buf[i]  =  0;
    return c_type;
}

int8_t stream_match(uint8_t** desc, uint8_t max, uint8_t* c) {
    uint8_t abs_min = 0; /* The initial value (before moving boundaries). */
    uint8_t cmp_idx = 0;
    uint8_t i;
    int8_t c_type   = 0;
    uint8_t min     = 0;

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
            c_type = (*gnext)(c);
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
            c_type = (*gnext)(c);
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
