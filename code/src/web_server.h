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
* @brief Identify and read a q-value parameter.
*
* It should be invoked after an Accept header has been identified on (and
* consumed from) the stream. Any LWS on the stream is discarded without
* affecting its output.

* This function is provided on the premise that interest lies in extracting only
* q-value parameters from the stream.
*
* @param qvalue
* @param c
* @returns One of:
*   - #OTHER
*   - #CRLF
*   - EOF
*/
static int8_t parse_header_param_qvalue(uint16_t* qvalue, uint8_t* c);

/**
* @brief Read a q-value from stream.
*
* According to <a href="http://tools.ietf.org/html/rfc2616#section-3.9">IETF RFC
* 2612 p.29</a>, a quality value is a short "floating point"
* number ranging from 0 up to 1 with, at most, three decimal points. The BNF is:
* @verbatim
qvalue = ( "0" [ "." 0*3DIGIT ] )
       | ( "1" [ "." 0*3("0") ] )@endverbatim
*
* The q-value is actually read from stream as an integer (per mil).
*
* @param[out] value The q-value read from stream.
* @param[in,out] c The first character to start parsing from and the last one
*   read.
* @returns One of:
*   - #OTHER
*   - EOF
*/
int8_t q_value(uint16_t* value, uint8_t* c);

/**
* @brief Advances the stream until all successive linear white space has been
* read.
*
* This function is mostly useful for removing spacing as well as header folding
* within a header body. A linear white space -- LWS, for short -- is defined by
* <a href="http://tools.ietf.org/html/rfc822#section-3.3">IETF RFC 288 p.10</a>,
* as: @verbatim 1*([CRLF] LWSP-char) @endverbatim
* ie, an optional CRLF sequence immediately followed by a SPACE or HTAB any
* amount of times. For clarification, a CRLF sequence followed by SPACE or HTAB,
* causes header folding and not its termination.
*
* @param[in,out] c The first character to start discarding from and the last one
*   read from the stream.
* @returns One of:
*   - #CRLF; for header termination.
*   - #OTHER; if any non-LWS has been read. That character will have been read
*       into @p c.
*   - EOF; if the end of stream has been reached.
*/
static int8_t discard_LWS(uint8_t* c);

/**
* @brief Discards the HTTP header-value parameter starting at @p c.
*
* Discards all characters on the stream until a ";" (end of current parameter),
* a "," (end of header-value), a CRLF (end of header) or EOF has occurred.
* In either delimiter ";" or ",", #OTHER is returned and the actual delimiter
* is found in @p c.
*
* Any quoted strings encountered along the way (and quoted-pairs therein) are
* rejected as well.
* Delimiters that occur within the boundaries of quoted strings are handled as
* ordinary text and do not terminate execution. Also, escaped double quotes
* (quoted-pair) are handled accordingly. It should be noted that although a "\"
* within a quoted string should be able to escape a CR, it is not permitted to
* do so, if that would cancel a CRLF sequence or header folding.
*
* @param[out] c The first character to start discarding from and the last one
*   read from the stream.
* @returns One of:
*   - #OTHER; if ";" or "," was read.
*   - #CRLF
*   - EOF
*/
static int8_t discard_param(uint8_t* c);

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
