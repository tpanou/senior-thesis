/**
@file
*/
#ifndef WEB_SERVER_H_INCL
#define WEB_SERVER_H_INCL

#include <inttypes.h>

/**
* @brief Array of server strings.
*
* Values include supported methods, header names and media ranges.
*/
static uint8_t* server_consts[] = {
    /* Methods, min: 0, max: 2 */
    "GET",
    "POST",
    /* Headers, min: 2, max: 5 */
    "accept",
    "content-length",
    "content-type",
    /* Media range, min: 5, max: 10 */
    "*/*",
    "application/*",
    "application/xml",
    "text/*",
    "text/html"
};

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
* @brief Convert character @p c to lower-case.
*/
#define TO_LOWER(c) if (!(c > 'Z' || c < 'A')) c |= 0x20

/**
* @brief The starting index in #server_consts of supported method literals.
*/
#define METHOD_MIN            0
#define METHOD_GET            0 /**< @brief Method @c GET. */
#define METHOD_POST           1 /**< @brief Method @c POST. */
/**
* @brief The upper-bound of method literals.
* It should be one unit greater than the index of the last method literal.
*/
#define METHOD_MAX            2

/**
* @brief The starting index in #server_consts of supported header literals.
*/
#define HEADER_MIN            2
#define HEADER_ACCEPT         2 /**< @brief Header @c Accept. */
#define HEADER_CONTENT_LENGTH 3 /**< @brief Header @c Content-Length. */
#define HEADER_CONTENT_TYPE   4 /**< @brief Header @c Content-Type. */
/**
* @brief The upper-bound of header literals.
* It should be one unit greater than the index of the last header literal.
*/
#define HEADER_MAX            5

/**
* @brief The starting index in #server_consts of supported media range literals.
*/
#define MIME_MIN              5
#define MIME_ANY              5 /**< @brief Media range "* / *". */
#define MIME_APP_ANY          6 /**< @brief Media range "application/any". */
#define MIME_APP_XML          7 /**< @brief Media range "application/xml". */
#define MIME_TEXT_ANY         8 /**< @brief Media range "text/ *". */
#define MIME_TEXT_HTML        9 /**< @brief Media range "text/html". */
/**
* @brief The upper-bound of media range literals.
* It should be one unit greater than the index of the last media range literal.
*/
#define MIME_MAX             10

/**
* @brief Find the closest match from an array of strings with the stream.
*
* Accepts an array of strings (descriptors) and compares the against the input
* stream. The descriptors should be sorted in ascending order beforehand, if
* necessary, and no duplicates should exist. Otherwise, the result is undefined.
*
* The return value is the index of the closest matching descriptor (which may or
* may not be a valid match -- more on this later); the value #OTHER, if it is
* certain no descriptor provides a match; or @c EOF if the end of the stream
* has been reached in the meantime.
* It should be noted that this function irreversibly consumes bytes from the
* input stream which are not reinstated back into it even if no match is found.
*
* More specifically, on each iteration a single character is extracted from the
* input stream and is compared against the known descriptors and, in particular,
* against the character at a position equal to the number of characters read
* (ie, the first character read is compared against the first character of all
* descriptors, etc).
*
* Initially, all the descriptors are included in the comparisons. However, as
* the iterations progress, descriptors that fail a match are omitted from the
* remaining iterations. Because the descriptors are given in ascending order,
* this is easily done by maintaining two moving boundaries.
*
* By the end of the iterations, the two boundaries have converged and specify
* a particular descriptor. That descriptor is a match if two conditions apply:
* - Its last character has been reached (null-character).
* - The input character that caused that last mismatch is an acceptable
* delimiter for this particular input.
*
* The former is readily determined whilst the latter, not. This is because this
* function is content-unaware and, thus, ignorant of what constitutes an
* acceptable delimiter. Consequently, it cannot fully determine whether the
* index returned actually corresponds to a full match; that is the
* responsibility of the caller. It can, however, determine with certainty when
* there is no match, in which case, it returns #OTHER.
*
* @param[in] desc Array with strings (descriptors).
* @param[in] min The lower boundary of @p desc; index of the first literal to
*   use in the comparisons.
* @param[in] max The upper boundary of @p desc; index of the last literal to use
*   in the comparisons, incremented by 1.
* @param[in,out] c The first character to compare against the strings and the
*   last one read from the stream.
* @returns One of:
*   - Index of @p desc with a possible match.
*   - #OTHER; on no match.
*   - EOF; if end of stream has been reached before hitting a match/mismatch.
*/
int stream_match(uint8_t** desc, uint8_t min, uint8_t max, uint8_t* c);

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
