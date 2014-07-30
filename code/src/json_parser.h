/**
* @file
* @brief A rudimentary JSON parser.
* @addtogroup json_parser JSON Parser
* @ingroup http_server
* @{
*
* @brief A rudimentary JSON parser.
*
* @section sec_json_parser_reason The reason
* JSON parsing is required as this data format is used as a lightweight
* container to receive the parameters needed to perform operations on the
* exposed resources of the device through the use of HTTP. These parameters are
* to be supplied as member names of a serialized JSON object, the latter of
* which would constitute the entity-body of the request itself. The main
* requirements for such a parser (other than recognising the JSON @c object
* value), are to:
*   - Parse input as it becomes available without the need to be provided a
*       buffer upon which to operate.
*   - Convert serialized data to appropriate data types for use by the
*       application, specifying which where set and which were erroneous.
*   - Produce as little overhead as possible (mainly as far as program space
*       consumption is concerned).
*
* It was, thus, deemed necessary to design and implement a specific JSON parser
* for this application.
*
* @section sec_json_parser_current_status Current status
* Only limited support for serialized JSON formatted data is provided, loosely
* based on \"<a href="tools.ietf.org/html/rfc7159">RFC 7159 - The JavaScript
* Object Notation (JSON) Data Interchange Format</a>\", as this implementation
* is not meant to be a full-fledged, general purpose parser; it is specifically
* designed to meet the current needs of the application with as little overhead
* as possible, even to the expense of conformance to that RFC.
*
* As a result, out of the seven defined JSON values (@c object, @c array, @c
* number, @c string, @c false, @c null and @c true), only objects, numbers and
* strings are recognised, liable to the following restrictions:
*   - Objects are not nested (ie, no object is set as a member value within
*       another object).
*   - Object members contain a key of type @c string. All strings begin and end
*       with quotation marks (RFC 7159 p.8).
*   - A predefined set of possible member names (tokens) is provided along with
*       their expected data type and storage memory (see param.h). Should the
*       parser come across a member name, type or size other than those in the
*       set, parsing stops.
*   - Numbers are unsigned integers of specified resolution (8- or 16-bit).
*   - Escaped Unicode characters in strings are not recognised. If present, they
*       are preserved as a plain text.
*
* Other than those restrictions, it is irrelevant to the parser whether any of
* the specified members (tokens) were present, the order in which, or how many
* times they have occurred. The caller may identify whether a member was
* specified by checking its corresponding status bits (see ParamValue#status_len
* in param.h). If set multiple times, the value from the last member occurrence
* is preserved.
*
* According to the specification, any number of insignificant white-space may
* occur around a structural character, the latter being a sequence of a
* white-space followed by a reserved character and another white-space (*RFC
* 7159 p.5*), eg:@verbatim
begin-object    = ws %x7B ws  ; { left curly bracket
@endverbatim
* None of the parser functions within this module will fail if either of the
* above specified leading or trailing white-space is not present.
*
* This module is separated into more specific functions, each responsible for a
* different section of a serialised JSON object. All of them are strict in the
* sense they fail fast upon any unexpected occurrence. This lack of leniency
* provides greater minimalism and, thus, requires less program space. Successful
* completion is indicated by a return value of @c 0, whereas failure, by a
* negative value (typically, #OTHER or #EOF). These functions, of course, use
* one another. Currently, should any one of them fail will cause the others to
* fail, as well; #json_parse() -- the root function -- returns that value.
* In case of an error, the stream will not be advanced any further. The
* character that caused the failure, though, will not be reinstated back into
* the stream.
*
* @section sec_json_parser_use Use
* The predominant function of this module is json_parse() which will initiate
* parsing on the input stream. To use it, one must first set the input stream.
* This is done by calling json_set_source() at least once, supplying a function
* address. One should not attempt to call json_parse() without setting a valid
* function pointer. Failing to do so would compromise the entire application.
*
* @subsection sec_json_parser_dependencies Dependencies
* In order for this component to be operable, some additional components are
* required, such as the ability to compare an incoming string from the stream
* against the specified set of tokens (stream match), and a number of functions
* to convert serialized data to variable values (integer parser, and string
* copy). These requirements are satisfied by the following functions of
* stream_util.h:
*   - stream_match(uint8_t** tokens, uint8_t max, uint8_t* c)
*   - parse_uint8(uint8_t* value, uint8_t* c)
*   - copy_until(uint8_t* buf, uint8_t delim, uint8_t max, uint8_t* c)
*
* Care must be taken than the input function of that module is set to the
* appropriate one *before* invoking json_parse().
*/

#ifndef JSON_PARSER_H_INCL
#define JSON_PARSER_H_INCL

#include "param.h"
#include "stream_util.h"

#include <inttypes.h>

/**
* @brief Represents the current stage during various levels of input parsing.
*/
enum JSONState {
    /** @brief An object opening section may have been found. */
    JSON_OBJECT_BEGIN,

    /** @brief An object closing section has been found. */
    JSON_OBJECT_END,

    /** @brief A member opening section may have been found. */
    JSON_MEMBER_BEGIN,

    /** @brief A member closing section has been found. */
    JSON_MEMBER_END,

    /** @brief A member key (token) start may have been found. */
    JSON_KEY_BEGIN,

    /** @brief A member key (token) end may have been found. */
    JSON_KEY_END,

    /** @brief A value opening section has been found. */
    JSON_VALUE_BEGIN,

    /** @brief A value has been parsed. */
    JSON_VALUE_END
};

/**
* @brief Check whether @p x is a JSON white-space character.
*
* @p x is compared against the four characters defined a white-space: space
* (0x20), horizontal tab (0x09), line feed (0x0A) and carriage return (0x0D).
* (*RFC 7159 p.5*)
*/
#define JSON_IS_WS(x)  (x == ' ' || x == '\t' || x == '\n' || x == '\r' )

/**
* @brief Sets the function that supplies this module with bytes from the input
* stream.
*
* Sets the value of #gnext.
* The provided function should accept a single byte address into which to store
* the next byte found on the stream. It should return @c 0 if the contents of
* that byte had been successfully updated or #EOF, if no more bytes are
* available on the stream for this particular processing cycle.
*
* It should give access to the serialized JSON object regardless of
* transformations that may have been applied to the actual input (eg,
* compression, chunked input, local buffering).
*
* Note that if any of the provided functions of this module are invoked without
* previously setting the function pointer will, in all probability, cause the
* application to fail.
*
* @param[in] input_source Pointer to input function.
*/
void json_set_source(int8_t (*input_source)(uint8_t*));

/**
* @brief Search the specified parameters on a JSON formatted input stream.
*
* For a list of limitations and features of the current implementation, refer to
* the details section.
*
* @param[in] tokens An array of parameter-tokens (strings) that are to be
*   searched on the stream.
* @param[in,out] values An array of #ParamValue that describe the semantics
*   around each string provided in @p tokens.
* @param[in] len The amount of elements in @p tokens and @c values (obviously,
*   the same for both).
* @returns One of:
*   - 0; if the stream was successfully parsed. The status bits of each
*       #ParamValue in @p values, provide details for each parameter.
*   - #OTHER; if parsing failed at any stage (eg, unexpected parameter-token).
*       If an unacceptable value was provided for an acceptable token, its
*       corresponding status bits are set (#PARAM_INVALID or #PARAM_TOO_LONG).
*   - #EOF; if end-of-stream occurred prematurely while parsing the serialized
*       input.
*/
int8_t json_parse(uint8_t** tokens, ParamValue* values, uint8_t len);

/**
* @brief Produce a serialised object of the provided parameters.
*
* @p tokens is an array of object keys that will be included in the serialised
* output. Each one will be enclosed in double quotes and followed by the value
* in the corresponding index of @p values. The conversion output depends on the
* specified #DataType. For #DTYPE_UINT, a fixed width sub-string of 5 characters
* is produced (which contains up to 5 digits and white-space padding).
* Currently, only 8-bit numbers are supported; in future versions, the size bits
* of @link ParamValue#status_len status_len@endlink would provide more control.
* For #DTYPE_STRING, the contents of @link ParamValue#data_ptr data_ptr@endlink
* are copied (within double quotes) until the first occurrence of a null-byte.
*
* A single white-space is produced around each structural character (such as
* braces { }, colon :, comma ,) (as specified in RFC 7159 p.5*).
*
* Currently, the output string is automatically flushed over the network module 
* This should not be the default behaviour, but was deemed appropriate at the
* current state.
*
* @param[in] tokens An array of parameter-tokens (strings) that are to be
*   used as object keys.
* @param[in] values An array of #ParamValue that describe the semantics around
*   each string provided in @p tokens (#DataType and, perhaps, size).
* @param[in] len The amount of elements in @p tokens and @c values (obviously,
*   the same for both).
*/
void json_serialise(uint8_t** tokens, ParamValue* values, uint8_t len);

/**
* @brief Advance the stream till a non-white-space character.
*
* In JSON,a white-space character is one of: space (0x20), horizontal tab
* (0x09), line feed (0x0A) and carriage return (0x0D) (*RFC 7159 p.5*).
*
* @param[in,out] c The first character to start parsing from and the last one
*   read from the stream. When called, if it is not a white-space, it
*   immediately returns.
* @returns One of:
*   - 0; if a non-white-space character was read from the stream.
*   - #EOF; if the end-of-stream has occurred.
*/
int8_t json_discard_WS(uint8_t* c);

/**
* @brief Parse the stream for a serialized JSON object.
*
* Nested objects are not supported at this time.
*
* When called, @p c should contain the left curly bracket '{' (0x7b) of the
* object to parse (`begin-object'). Upon normal completion (return value @c 0),
* the character pointed to by @c p is its corresponding right curly bracket '}'
* (0x7d) (`end-object').
*
* This function uses #json_parse_member() which in turn uses
* #json_parse_value(). As stated before, they are all strict against unexpected
* occurrences. As far as this function is concerned, it means than all objects
* must start and end with a pair of curly brackets, optionally containing any
* number of members separated with *one* value-separator (*RFC 7159 p.6*):
* @verbatim
object = begin-object [ member *( value-separator member ) ]
         end-object

member = string name-separator value@endverbatim
*
* What is more important, though, is that a @c member's key is a @c string, and
* all strings start and end within quotation-marks (*RFC 7159 p.8*). As a
* result, this function will fail (return value @c #OTHER) should a key be
* specified without quotation-marks.
*
* Only keys specified in @p info may be accepted, the order of which is
* irrelevant (see #json_parse_member()).
*
* @param[in,out] info Provides the acceptable keys and a additional information
*   (for an explanation, see #ParamInfo).
* @param[in,out] c The first character to start parsing from and the last one
*   read from the stream. Upon invocation, it should point to '{' (0x7b).
* @returns One of:
*   - @c 0; if the object was successfully parsed and no errors have occurred.
*   - #OTHER; if an invalid character has occurred at any stage during
*       processing.
*   - #EOF; if end-of-stream was reached at any point.
*/
static int8_t json_parse_object(ParamInfo* info, uint8_t* c);

/**
* @brief Parses the stream for a member whose key is one of those found in @p
* info.
*
* When calling this, @p c should point to '"' (double quote) to designate the
* start of a key (keys are defined as strings and are, thus, enclosed within
* quotation marks). The input stream is matched against the values included in
* @p info (member @link #ParamInfo::tokens tokens@endlink) using the externally
* provided
* stream_match()
* function in an attempt to locate one of those keys/tokens. If it succeeds, the
* return value (status) of this function depends on the outcome of parsing the
* identified key's value by #json_parse_value(). On the contrary, if it happens
* upon a key that is not included in @p info, the operation stops (fails) and
* #OTHER is returned.
*
* @param[in,out] info Provides the acceptable keys and a additional information
*   (for an explanation, see #ParamInfo).
* @param[in,out] c The first character to start parsing from and the last one
*   read from the stream. Upon invocation, it should point to '"' (string).
* @returns One of:
*   - @c 0; if a member (key/value pair) with a key defined in @p info was
*       successfully parsed.
*   - #OTHER; if an invalid character has occurred at any stage during
*       processing.
*   - #EOF; if end-of-stream was reached at any point.
*/
static int8_t json_parse_member(ParamInfo* info, uint8_t* c);

/**
* @brief Parse the stream for a value of type and size defined by @p pvalue.
*
* @c pvalue.@link ParamValue::type type@endlink is used to determine the
* appropriate value parser to call (currently, one of the externally supplied
* #parse_uint8() or #copy_until()). According to the return value of those
* functions, the two most-significant bits in @c
* pvalue.@link ParamValue::status_len status_len@endlink are set to one of
* #PARAM_INVALID, #PARAM_TOO_LONG or #PARAM_VALID. In case #PARAM_VALID is set,
* the actual value can be read from @c
* pvalue.@link ParamValue::data_ptr data_ptr@endlink. Do note that @c
* pvalue.@link ParamValue::data_ptr data_ptr@endlink is *not* allocated within
* this function but *should* have been set to point to a valid memory location
* *before* the execution of this function. For details, see #ParamValue.
*
* Also note that even though the status of @c
* pvalue.@link ParamValue::status_len status_len@endlink may have been set to
* #PARAM_INVALID or #PARAM_TOO_LONG, the data pointed to by @c
* pvalue.@link ParamValue::data_ptr data_ptr@endlink
* may have been altered during processing. In any case, they should still be
* considered invalid.
*
* @param[in,out] pvalue Provides the type and size of the acceptable value as
*   well as access to its storage memory (for an explanation, see #ParamValue).
* @param[in,out] c The first character to start parsing from and the last one
*   read from the stream.
* @returns One of:
*   - @c 0; if a value in accordance to @p pvalue has been read from the stream
*       and stored into pvalue.@link ParamValue::data_ptr data_ptr@endlink.
*   - #OTHER; if an invalid character has occurred at any stage during
*       processing. This will also be reflected by #PARAM_INVALID being set into
*       pvalue.@link ParamValue::status_len status_len@endlink.
*   - #EOF; if end-of-stream was reached at any point.
*/
static int8_t json_parse_value(ParamValue* pvalue, uint8_t* c);

#endif /* JSON_PARSER_H_INCL */
/** @} */
