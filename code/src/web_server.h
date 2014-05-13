/**
@file
*/
#ifndef WEB_SERVER_H_INCL
#define WEB_SERVER_H_INCL

#include <inttypes.h>

/**
* @brief Indicates a CRLF sequence.
*/
#define CRLF    -4

/**
* @brief Indicates any character (based on the context of its use).
*/
#define OTHER   -5

#ifndef EOF
#define EOF     -1
#endif

/**
* @brief Checks whether there is a LWS on stream starting with @p c.
*
* A linear white space is an LWSP-char (ie, @c SPACE or @c HTAB) optionally
* preceded by a CRLF sequence.
*
* This function does not consume bytes from the stream. It simple peeks forward.
*
* @param c The first character to use in the comparisons.
* @returns @c 1, if true; @c 0, otherwise.
*/
int8_t is_LWS(uint8_t c);

/**
* @brief Checks whether there is a CRLF sequence on stream starting with @p c.
*
* It returns @c 0 (ie, false), if there is a CRLF sequence followed by an @c
* LWSP-char which, semantically, is a header folding.
*
* This function does not consume bytes from the stream. It simply peeks forward.
*
* @param c The first character to use in the comparisons.
* @returns @c 1, if true; @c 0, otherwise.
*/
int8_t is_CRLF(uint8_t c);

#endif /* WEB_SERVER_H_INCL */
