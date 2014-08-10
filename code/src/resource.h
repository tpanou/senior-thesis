/**
* @file
* @addtogroup resource Server Resource
* @ingroup http_server
*/

#ifndef RESOURCE_H_INCL
#define RESOURCE_H_INCL

struct HTTPRequest;

#include "param.h"
#include "defs.h"

#include <inttypes.h>

/**
* @brief Specification of methods that trigger a particular callback function.
*
* The callback function receives a pointer to the #HTTPRequest representation of
* the current request and may work with parsers to identify data on the input
* stream and produce an appropriate response. It is safe to assume the callback
* is only executed if the requested method (HTTPRequest#method) is applicable
* to this particular instance (ie, it has a corresponding bit set in
* @link ResourceHandler::methods methods@endlink).
* Note that @link ResourceHandler::methods methods@endlink is the bit-wise OR
* result of #MethodFlag values while #HTTPRequest::method is an *index* number
* of a #server_consts string literal.
*/
typedef struct ResourceHandler {
    /**
    * @brief Bit-wise-OR'ed #MethodFlag values representing the acceptable
    * methods for this resource. */
    uint8_t methods;

    /** @brief Handler callback for this particular path. */
    void (*call)(struct HTTPRequest*);
} ResourceHandler;

/**
* @brief Container of query parameters and their values.
*
* This is used by the http_parser module <http_parser.h> to identify the
* acceptable parameters for a particular URI and method combination. It is also
* set to contain their value as they are found on the stream. It is imperative
* most members be set up before use. Generally, the necessary initialisation is
* performed by rsrc_inform().
*/
typedef struct QueryString {
    /**
    * @brief Array of acceptable query parameter names.
    *
    * The items of this array relate to the items of .values on a one-on-one
    * basis; the string found in the corresponding index of .values is the value
    * of the parameter token found at the same index in this array.
    *
    * Typically, the acceptable parameters (tokens) are specified to the
    * http_parser module *after* the targeted resource of the request has been
    * found to match one of ServerSettings#rsrc_tokens. rsrc_inform() may be
    * used to update the contents of this member at that time.
    */
    uint8_t* tokens[QUERY_PARAM_LEN];

    /**
    * @brief Values for the parameters found in .tokens.
    *
    * The items of this array relate to the items of .tokens on a one-on-one
    * basis; the string at any index of this array is the value that has been
    * specified in the query string for the parameter token that resides at the
    * same index in array .tokens. @c NULL denotes that the parameter was not
    * found in the query string. */
    uint8_t* values[QUERY_PARAM_LEN];

    /**
    * @brief The permissible number of parameters for a particular resource.
    *
    * Typically, the acceptable parameters are specified *after* the resource of
    * the request has been identified to match one of
    * ServerSettings#rsrc_tokens. rsrc_inform() may be used to update the
    * contents of this member at that time.
    * This represents the maximum permissible number of parameters for a
    * particular resource which may or may not be equal to the size of arrays
    * .tokens and .values.
    *
    * A value of @c 0 denotes that no parameters are expected in the query
    * string.
    */
    uint8_t   count;

    /**
    * @brief Stores query parameter tokens and values.
    *
    * String pointers in .tokens and .values are set to addresses found within
    * this buffer or, alternatively, @c NULL (in case of .values).
    */
    uint8_t  buf[QUERY_BUF_LEN];

    /** @brief Offset in .buf to write to next. */
    uint16_t  buf_i;

    /** @brief The size of .buf. */
    uint16_t  buf_len;
} QueryString;

/**
* @brief Initialise the Resource module.
*
* This basically registers the resource tokens and handler function to the HTTP
* server so that they may be invoked every time a resource token is matched.
*
* It suffices to call this only once; even if a particular handler is replaced
* by another, there is no need for an update.
*/
void rsrc_init();

/**
* @brief Specify the parser to be used in any future handler invocations.
*
* This should be called every time a different input format is available on the
* stream since it is the parser that grants the handlers access to the contained
* values. Obviously, this should be done *before* triggering a handler. The
* provided parser function should comply with the param.h module.
*
* This function is used by the HTTP server whenever it is deemed necessary.
*
* @param[in] parser Function pointer to the appropriate parser to use.
*/
void rsrc_set_parser(int8_t (*parser)(uint8_t**, ParamValue*, uint8_t len));

/**
* @brief Specify the serialising function for any future handler invocations.
*
* This should be called every time a different output format is to be produced.
* Obviously, this should be done *before* triggering a handler. The provided
* parser function should comply with the param.h module.
*
* This function is used by the HTTP server whenever it is deemed necessary.
*
* @param[in] serialiser Function pointer to the appropriate serialising function
*   to use.
*/
void rsrc_set_serial(void (*serialiser)(uint8_t**, ParamValue*, uint8_t len, uint8_t ctr));

/**
* @brief Register a @p handler for specific @p methods on a particular @p uri.
*
* Note that URIs not registered at all, will cause the server to return a 404
* (Not Found) response to the requester entity. To replace a previously set
* handler with a new one, simply call this function with the appropriate
* settings.
*
* Also, see #MethodFlag and #ResourceHandler.
*
* @param[in] uri A value within #rsrc_handlers describing one of the
*   available server resources.
* @param[in] methods Any bit-wise OR combination of acceptable #MethodFlag bits
*   for @p uri.
* @param[in] handler The function to call when an acceptable method for this @p
*   uri is received.
*/
void rsrc_set_handler(uint8_t uri,
                      uint8_t methods,
                      void (*handler)(struct HTTPRequest*));

/**
* @brief Update @p req with URI-method specific options.
*
* Apart from server-wide options that are readily available, options closely
* linked to a particular resource may only be provided once such a resource has
* been identified. This is done to conserve main memory that would otherwise be
* consumed for all resource-specific tokens, regardless of them being used or
* not. One such example is the query string parameters. Only the appropriate
* tokens are loaded into main memory for a particular URI-method pair.
*
* Currently, this function is a wrapper around rsrc_get_qparam().
*
* @param[in,out] req An #HTTPRequest variable that has its .uri and .method
*   members set. The appropriate members of @p req will be initialised.
*/
void rsrc_inform(struct HTTPRequest* req);

/**
* @brief Update the #QueryString of @p req according to .uri and .method.
*
* It does the following:
* - The appropriate query parameter tokens are loaded into main memory, and in
*   particular, in @link QueryString#buf query.buf@endlink.
* - @link QueryString#count query.count@endlink is updated to reflect the
*   permissible amount of parameters, the tokens of which can be found in
*   @link QueryString#tokens query.tokens@endlink.
* - Indices in @link QueryString#values query.values@endlink equal to the amount
*   of permissible parameters are set to @c NULL to indicate they have not yet
*   been given a value.
* - @link QueryString#buf_i query.buf_i@endlink is set to an offset within
*   @link QueryString#buf query.buf@endlink, which the first parameter value may
*   be written at.
* - @link QueryString#buf_len query.buf_len@endlink is set to #QUERY_BUF_LEN.
*
* @param[in,out] req Accepts an #HTTPRequest variable that has its
*   @link HTTPRequest#uri uri@endlink and
*   @link HTTPRequest#method method@endlink members set. If no parameters are
*   applicable to them, @link QueryString#count query.count@endlink will be set
*   to @c 0 whereas @link QueryString#buf_i query.buf_i@endlink will be equal to
*   @link QueryString#buf_len query.buf_len@endlink (meaning the buffer is full
*   and should not be written to).
*/
static inline void rsrc_get_qparam(struct HTTPRequest* req);

#endif /* RESOURCE_H_INCL */
