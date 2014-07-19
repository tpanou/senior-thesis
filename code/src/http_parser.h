/**
* @file
* @addtogroup http_parser HTTP Parser
* @ingroup http_server
* @{
*/
#ifndef WEB_SERVER_H_INCL
#define WEB_SERVER_H_INCL

#include <inttypes.h>

/**
* @brief A representation of an HTTP message.
*/
typedef struct {
    /** @brief Value representing the method of the request. */
    uint8_t method;

    /** @brief Value representing the URI of the request line. */
    uint8_t uri;

    /** @brief The major number of the HTTP version of the message. */
    uint8_t v_major;

    /** @brief The minor number of the HTTP version of the message. */
    uint8_t v_minor;

    /** @brief Value representing the accept media range of the request. */
    int8_t accept;

    /** @brief Value representing the transfer encoding of the message. */
    uint8_t transfer_encoding;

    /** @brief Value representing the content type of the message. */
    uint8_t content_type;

    /** @brief The length (in octets) of the message. */
    uint16_t content_length;
} HTTPRequest;

/**
* @brief Array of server strings.
*
* Values include supported methods, header names and media ranges.
*/
static uint8_t* server_consts[] = {
    /* Methods, min: 0, max: 2 */
    "get",
    "put",
    /* Headers, min: 2, max: 6 */
    "accept",
    "content-length",
    "content-type",
    "transfer-encoding",
    /* Media range, min: 6, max: 11 */
    "*/*",
    "application/*",
    "application/xml",
    "text/*",
    "text/html",
    /* HTTP tokens, index: 11, 12 */
    "http",
    "http://",
    /* Transfer-codings, min: 13, max: 15 */
    "chunked",
    "identity",
    /* Absolute paths, min: 15, max: 18 */
    "*",
    "/",
    "/index.html"
};

/**
* @brief The name of the server (value of `Host' header and part of absolute
* URIs).
*
* It is not necessary to be an IP address. Use set_host_name() to update its
* value at any time. It should be noted that no trailing slash should ever be
* appended.
*/
static uint8_t host_name[] = "000.000.000.000";

/**
* @brief The listening port of the server; defaults to 80.
*
* Use set_host_port() to update its value.
*/
static uint8_t host_port[6] = "80";

/**
* @brief Denotes whether a chunked message is already in process.
*/
static uint8_t is_chunk_on  = 0;

/**
* @brief Amount of total bytes to read from the current chunk.
*/
static uint16_t chunk_len   = 0;

/**
* @brief Amount of bytes read from the current chunk.
*/
static uint16_t chunk_pos   = 0;

/**
* @brief Indicates a CRLF sequence.
*/
#define CRLF    -4

#ifndef EOF
/**
* @brief Indicates that the End-of-File has been reached.
*/
#define EOF     -1
#endif

/**
* @brief The starting index in #server_consts of supported method literals.
*/
#define METHOD_MIN            0
#define METHOD_GET            0 /**< @brief Method @c GET. */
#define METHOD_PUT            1 /**< @brief Method @c PUT. */
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
#define HEADER_TRANSFER_ENC   5 /**< @brief Header @c Transfer-Encoding. */
/**
* @brief The upper-bound of header literals.
* It should be one unit greater than the index of the last header literal.
*/
#define HEADER_MAX            6

/**
* @brief The starting index in #server_consts of supported media range literals.
*/
#define MIME_MIN              6
#define MIME_ANY              6 /**< @brief Media range "* / *". */
#define MIME_APP_ANY          7 /**< @brief Media range "application/any". */
#define MIME_APP_XML          8 /**< @brief Media range "application/xml". */
#define MIME_TEXT_ANY         9 /**< @brief Media range "text/ *". */
#define MIME_TEXT_HTML       10 /**< @brief Media range "text/html". */
/**
* @brief The upper-bound of media range literals.
* It should be one unit greater than the index of the last media range literal.
*/
#define MIME_MAX             11

#define HTTP_SCHEME          11 /**< @brief HTTP literal. */
#define HTTP_SCHEME_S        12 /**< @brief HTTP scheme with separator. */

/**
* @brief The starting index in #server_consts of available transfer-coding.
*/
#define TRANSFER_COD_MIN     13
#define TRANSFER_COD_CHUNK   13 /**< @brief Chunked transfer-coding. */
#define TRANSFER_COD_IDENT   14 /**< @brief Identity transfer-conding. */
/**
* @brief The upper-bound of transfer-coding literals.
* It should be one unit greater than the index of the last transfer-coding
* literal.
*/
#define TRANSFER_COD_MAX     18

/**
* @brief The starting index in #server_consts of available endpoints.
*/
#define URI_MIN              15
#define URI_SERVER           15 /**< @brief Server "*". */
#define URI_ROOT             16 /**< @brief Server root "/". */
#define URI_INDEX_HTML       17 /**< @brief Endpoint "/index.html". */
/**
* @brief The upper-bound of endpoint literals.
* It should be one unit greater than the index of the last endpoint literal.
*/
#define URI_MAX              18

/**
* @brief Describes an unsupported transfer-coding value or combination thereof.
*
* This is not a #server_consts index.
*/
#define TRANSFER_COD_OTHER    1

/**
* @brief Convert and set a IP address as the host name of the HTTP server.
*
* This function is an alternative to set_host_name(). It receives an array of
* four bytes and converts them to an equivalent IP address string
* (null-terminated).
*
* More specifically, for each address byte, an initial position is estimated and
* then incremented dependent on the number of its digits. Then, a character is
* passed into #host_name for each of its digits (using modulo and quotient).
*
* @param[in] ip A four-byte array of an IP address.
*/
void set_host_name_ip(uint8_t* ip);

/**
* @brief Parse the input stream and return an #HTTPRequest representation of it.
*
* The contents of the input stream are considered to be an HTTP request. First,
* the request line is parsed (see #parse_request_line()). Then, the headers
* follow (see #parse_headers()). Headers that are not supported are ignored.
*
* Generally, the input stream should be left in its initial (intact) state when
* calling this function. Upon completion, the next byte read from the stream
* with #s_next() is the first byte of the message-body, if one exists.
*
* In case #HTTPRequest.transer_encoding is set to #TRANSFER_COD_CHUNK, use of
* #c_next() gives direct (transparent) access to the entity-body and should be
* preferred over #s_next().
*
* @returns An #HTTPRequest representation of the HTTP request found in the input
*   stream.
*/
HTTPRequest http_parse_request();

/**
* @brief Extract method, request URI and HTTP version of the request line from
* the stream.
*
* This function is a congregate of the functions stream_match(), parse_uri() and
* parse_http_version().
*
* @param[out] req HTTPRequest variable to be updated with values found on
*   stream. Those members are:
*   @link HTTPRequest::method method@endlink,
*   @link HTTPRequest::uri uri@endlink,
*   @link HTTPRequest::v_major v_major@endlink and
*   @link HTTPRequest::v_minor v_minor@endlink.
* @param[in,out] c The first character to start parsing from and the last one
*   read from the stream.
* @returns One of:
*   - CRLF
*   - EOF
*/
int8_t parse_request_line(HTTPRequest* req, uint8_t* c);

/**
* @brief Read the specified transfer-coding from stream.
*
* Only `chunked' and `identity' are currently supported. Should any other, or
* combination thereof, be specified, #TRANSFER_COD_OTHER shall be returned.
*
* As the `Transfer-Encoding' and `TE' headers are a list, this function may be
* invoked more than once for each.
*
* @param[in,out] value One of #TRANSFER_COD_CHUNK, #TRANSFER_COD_IDENT or
*   #TRANSFER_COD_OTHER.
* @param[in,out] c The first character to start parsing from and the last one
*   read from the stream, typically @c LF.
* @returns One of:
*   - CRLF
*   - EOF
*/
int8_t parse_header_transfer_coding(uint8_t* value, uint8_t* c);

/**
* @brief Read HTTP major and minor version numbers from stream.
*
* @param[out] req HTTPRequest variable to be updated with the message's HTTP
*   version. Members are: @link HTTPRequest::v_major v_major@endlink and
*   @link HTTPRequest::v_minor v_minor@endlink.
*   stream. Which members are actually update depends on what is available.
* @param[in,out] c The first character to start parsing from and the last one
*   read from the stream.
* @returns EOF on end of stream; @c 0 or #OTHER, otherwise.
*/
int8_t parse_http_version(HTTPRequest* req, uint8_t* c);

/**
* @brief Populates @p req with header values found on the stream.
*
* Is uses stream_match() to identify headers at the beginning of each line and
* then
* forwards the rest of the line to the appropriate header-body handler for that
* header. Unsupported headers are simply discarded.
* It terminates on either EOF or a double CRLF (ie, an empty line).
*
* @param[out] req HTTPRequest variable to be updated with values found on
*   stream. Which members are actually update depends on what is available.
* @param[in,out] c The first character to start parsing from and the last one
*   read from the stream.
* @returns One of:
*   - #CRLF; if an empty line was read. The double CRLF sequence will be dropped
*       from the stream and the last LF returned in @p c.
*   - EOF. Normally, this should not occur in the header section.
*/
int8_t parse_headers(HTTPRequest* req, uint8_t* c);

/**
* @brief Parse Accept header body-value is search of media ranges.
*
* This function traverses and consumes the stream from its current position up
* to a CRLF or EOF in an attempt to identify a supported media range (ie, one
* that is included in the #server_consts).
*
* For any identified media range, it also provides its q-value. If more than one
* q-value is specified for a particular media range, the last one is preserved.
* For details on the conversion refer to q_value().
*
* According to <a href="http://tools.ietf.org/html/rfc2616#section-14.1">IETF
* RFC 2616 p.100</a>, the Accept header is a list and, so, it may
* contain more than one media range. The one with the highest q-value is
* returned.
*
* Since the Accept header is a list, it may appear more than once in a request.
* Should such an occasion arise, the @p media_range and @p qvalue returned by a
* previous call to this function can be used in its new invocation. This way,
* they will be taken into consideration while parsing the header-body.
* Initially, a non-acceptable index of #server_consts and 0, respectively,
* should suffice.
*
* @param[in,out] media_range Index of #server_consts that corresponds to
*   the highest, so far, qvalue-ranking media range. Upon first invocation, it
*   should contain a known invalid value.
* @param[in,out] qvalue The q-value of the @p media_range. On the first
*   invocation, it should contain a value of 0.
* @param[out] c The last character read from the stream.
* @returns One of:
*   - #CRLF
*   - EOF
*/
int parse_header_accept(int8_t* media_range, uint16_t* qvalue, uint8_t* c);

/**
* @brief Match input from stream against the available server endpoints.
*
* Absolute paths are recognised as well as absolute URIs; use set_host_name() to
* provide the server name or IP address. In the case of an absolute URI, the
* port should match that of the server, if one is specified in the request; see
* set_host_port().
*
* This function is in accordance with
* <a href="http://tools.ietf.org/html/rfc2616#section-5.1.2">IETF RFC 2616
* p.37</a> in that if the Request-URI is encoded using the "% HEX HEX" encoding,
* it decodes the corresponding characters. To do so, it uses stream_match_ext()
* to be informed of cases that contain a percent sign. If the encoding is valid
* (ie, a hexadecimal number), the decoded character is fed back into
* stream_match_ext() and the operation continues as normal.
*
* @param[out] req HTTPRequest variable to be updated with the endpoint value
*   found on (@link HTTPRequest::uri uri@endlink).
* @param[in,out] c The first character to start parsing from and the last one
*   read from the stream.
* @returns One of:
*   - CRLF
*   - EOF
*/
static int8_t parse_uri(HTTPRequest* req, uint8_t* c);

/**
* @brief Matches the current name of the server (host) against the stream.
*
* To see how to set the host name of the server, refer to set_host_name().
*
* @param[in,out] c The first character to start comparing from and the last one
*   read from the stream.
* @returns One of:
*   - @c 0; if @verbatim "http:" "//" host @endverbatim was matched.
*   - #OTHER; if either the scheme or the host name failed to match.
*   - EOF; if the end of stream was reach in the meantime.
*/
static int8_t parse_host(uint8_t* c);

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
* @brief Read the next byte from a chunked HTTP message into @p c.
*
* In case the bytes of a particular chunk have been depleted, the first byte of
* the next chunk (if one exists) is returned. This function should only be
* called if a `Tranfser-Encoding' header with a value of `@c chunked' has been
* specified.
*
* @param[out] c The character read from the stream.
* @returns @c 0 on a successful read; @c EOF, otherwise.
*/
int8_t c_next(uint8_t* c);

/**
* @brief Parse the size of the next chunk.
*
* This function is called by front-end functions.
*
* According to <a href="http://tools.ietf.org/html/rfc2616#section-3.6.1">IETC
* RFC 2616 p.26</a> "All HTTP/1.1 applications MUST be able to receive and
* decode the @c chunked transfer-coding, and MUST ignore chunk extensions they
* do not understand". Only the size is parsed as no chunk extensions are
* currently supported. An extract of the BNF is as follows: @verbatim
Chunked-Body   = *chunk
                 last-chunk
                 trailer
                 CRLF

chunk          = chunk-size [ chunk-extension ] CRLF
                 chunk-data CRLF
chunk-size     = 1*HEX
last-chunk     = 1*("0") [ chunk-extension ] CRLF

chunk-data     = chunk-size(OCTET)@endverbatim
*
* The amount of octets of each chunk is given as a hexadecimal number (excluding
* the CRLF sequence). The last chunk size should evaluate to 0. The trailer is
* also ignored because, as stated by the same source as above, it consists
* entirely of optional metadata that may be discarded before reaching the
* client (in this case, they are discarded by the client).
*
* @param[in,out] c The first character to start parsing from and the last one
*   read from the stream.
* @returns @c EOF on end of stream; @c 0, otherwise.
*/
static int8_t update_chunk(uint8_t* c);

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
* @brief Discard everything up to an empty line (ie, two CRLF sequence in
* succession).
*
* @param[in,out] c The first character to start discarding from and the last one
*   read from the stream.
* @returns One of:
*   - #CRLF; on an empty line.
*   - @c EOF; on end-of-stream.
*/
static int8_t discard_to_line(uint8_t* c);

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

/**
* @brief Checks whether there is a CRLF sequence on stream starting with @p c.
*
* Unlike is_CRLF(), this function is satisfied with a CRLF sequence no matter
* what follows next, be it a LWS-char or otherwise.
*
* This function does not consume bytes from the stream. It simply peeks forward.
*
* @param c The first character to use in the comparisons.
* @returns @c 1, if true; @c 0, otherwise.
*/
int8_t is_c_CRLF(uint8_t c);

#endif /* WEB_SERVER_H_INCL */
/** @} */
