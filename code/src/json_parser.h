/**
* @file
* @addtogroup json_parser JSON Parser
* @ingroup http_server
* @{
*
* TODO General description
*
* Currently, only limited support for serialized JSON streams is provided,
* loosely based on \"<a href="tools.ietf.org/html/rfc7159">RFC 7159 - The
* JavaScript Object Notation (JSON) Data Interchange Format</a>\".
* More specifically,
*
* TODO Specify what *is* supported. Basically, objects (not nested) with values
*   of type integer (unsigned) and string, of the specified keys and
*   resolution/size.
*
* According to the specification, any number of insignificant white-space may
* occur around a structural character the latter being a sequence of a
* white-space followed by a reserved character and another white-space (*RFC
* 7159 p.5*), eg:@verbatim
begin-object    = ws %x7B ws  ; { left curly bracket
@endverbatim
* None of the parser functions will fail if either of the above specified
* leading or trailing white-space is not present.
*
* TODO Complete the following:
* The predominant function is #json_parse() which
*
* TODO Change this "All the provided…" to something more appropriate, since most
*   of them are static (global within this compilation unit):
* All the provided json_parse_* functions are strict in the sense they fail fast
* upon any unexpected occurrence. This lack of leniency provides greater
* minimalism and, thus, requires less program space. Successful completion is
* indicated by a return value of @c 0, whereas failure, by a negative value
* (typically, #OTHER or #EOF). These functions, of course, use one another.
* Currently, should any one of them fail will cause the others to fail, as well;
* #json_parse() will return that value.
*
* Members contain a key of type @c string. All strings begin and end with
* quotation marks (RFC 7159 p.8). See #json_parse_object().
*
* TODO Specify what are the required *extern* components (for instance, function
*   to read one character at a time and a string matcher) and the overall
*   architecture, which brings to the following:
*
* In order for this component to be operable it needs to be provided with the
* following:
*   - @code int8_t (*gnext)(uint8_t*)@endcode
*   - @code int8_t (*gmatch)(uint8_t**, uint8_t, uint8_t, uint8_t*)@endcode
*
*/

/*
* TODO Include definition of #OTHER (and perhaps #EOF), *after* they have been
*   defined in appropriate files.
*/

#ifndef JSON_PARSER_H_INCL
#define JSON_PARSER_H_INCL

enum JSONStage {
    JSON_OBJECT_BEGIN,
    JSON_OBJECT_END,
    JSON_MEMBER_BEGIN,
    JSON_MEMBER_END,
    JSON_KEY_BEGIN,
    JSON_KEY_END,
    JSON_VALUE_BEGIN,
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
