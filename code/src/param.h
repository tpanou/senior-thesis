/**
* @file
*/

/**
* @brief Parameter value data types.
*
* Mostly needed to specify the data type for a particular parameter (and load
* the appropriate value parser).
*
* Currently, only unsigned integers and strings are specified.
*/
typedef enum {
    /** @brief Unsigned integer (8- or 16-bit). */
    DTYPE_UINT,

    /** @brief Null-terminated character sequence. */
    DTYPE_STRING
} DataType;

/**
* @brief Controls parsing for the value of a particular parameter-token.
*/
typedef struct {
    /** @brief Data type for the value of this token. One of #DataType. */
    DataType    type;

    /**
    * @brief Memory location where a valid value should be placed at.
    *
    * This is set by the caller and updated by the parser. That is, the caller
    * sets the address, perhaps one of its local variables, and the parser
    * functions update its contents to match the value parsed from the stream.
    */
    void*       data_ptr;

    /**
    * @brief Specifies the length and the status of a parameter.
    *
    * The meaning of this member is two-fold. The two most significant bits
    * determine the status of @link ParamValue::data_ptr data_ptr@endlink, while
    * the rest, the allowable size for a parsed value.
    *
    * The status bits (bits @c 7 and @c 6) indicate the outcome for this
    * particular parameter, such as whether a valid value (#PARAM_VALID), an
    * inappropriate character (#PARAM_INVALID) or too many characters
    * (#PARAM_TOO_LONG) have been discovered on the stream. Their value should
    * be reset to @c 00 (#PARAM_NOT_SET) *before* any parsing takes place.
    * Setting these bits is, typically, performed by the assigned parser.
    *
    * The size bits (bits @c 5 through @c 0) directly affect the maximum number
    * of characters parsed for this particular parameter and also imply the size
    * of the data storage pointed to by @c data_ptr. Their exact meaning depends
    * on @link ParamValue::type type@endlink. For instance, if @c type is set to
    * #DTYPE_UINT and the size bits are set to 0x08, the assigned parser should
    * parse the stream for an integer value of up to 255, storing it at the
    * memory location pointed to by @c data_ptr. Note, that in case of
    * #DTYPE_UINT, not all size values would make sense (eg, 10). Ultimately, it
    * depends on the parser.
    *
    * As another example, setting @c type to #DTYPE_STRING and the size bits to
    * 0x10, direct the assigned parser to copy *up to 15* bytes from the input
    * stream to the buffer pointed to by @c data_ptr followed by one final
    * null-byte. Whether 15 or less bytes are actually copied depends on the
    * semantics supported by the parser and what, for example, constitutes an
    * appropriate delimiter.
    *
    * In order to ensure smooth operation between successive invocations and to
    * avoid redundant initializations, the parsers should make certain to never
    * alter the contents of the size bits.
    */
    uint8_t     status_len;
} ParamValue;

/**
* @brief Wrapper of parameter-tokens and their #ParamValue.
*/
typedef struct {
    /** @brief Array of parameter tokens to match against the stream. */
    uint8_t**   tokens;

    /** @brief Array of #ParamValue corresponding to @c tokens. */
    ParamValue* values;

    /**
    * @brief The size of @c tokens and @c values.
    *
    * Normally, @c tokens and @c values are of the same size (since they are
    * related one-on-one).
    */
    uint8_t     len;
} ParamInfo;

/**
* @brief Extract token status out of @link ParamValue::status_len@endlink.
*
* Once applied to @c status_len, it preserves just the part that corresponds to
* the status. The result may then be compared against #PARAM_NOT_SET,
* #PARAM_VALID, #PARAM_INVALID and/or #PARAM_TOO_LONG. It may also be used to
* reset the status (to indicate it has not been set yet). This is done
* automatically by json_parse() for the tokens supplied for parsing.
*/
#define PARAM_STATUS_MASK  (0xC0)

/**
* @brief Indicates that this token has not occurred on the stream.
*
* This only holds true if the corresponding bits of @c status_len have initially
* been set to this particular value. This is done automatically by json_parse()
* for the tokens supplied for parsing.
*
* Note that this value refers only to the two most significant bits of @c
* status_len. For details, see #ParamValue.
*
* Also, see #PARAM_STATUS_MASK, #PARAM_VALID, #PARAM_INVALID, #PARAM_TOO_LONG.
*/
#define PARAM_NOT_SET      (0x00)

/**
* @brief Indicates that an invalid character occurred during parsing for a
* value.
*
* Note that this value refers only to the two most significant bits of @c
* status_len. For details, see #ParamValue.
*
* Also, see #PARAM_STATUS_MASK, #PARAM_NOT_SET, #PARAM_VALID, #PARAM_TOO_LONG.
*/
#define PARAM_INVALID      (0x40)

/**
* @brief Indicates that too many characters have been read during parsing for a
* value.
*
* Note that this value refers only to the two most significant bits of @c
* status_len. For details, see #ParamValue.
*
* Also, see #PARAM_STATUS_MASK, #PARAM_NOT_SET, #PARAM_VALID, #PARAM_INVALID.
*/
#define PARAM_TOO_LONG     (0x80)

/**
* @brief Indicates that a valid value has been parsed from the stream.
*
* The value is stored into @link ParamValue::data_ptr@endlink.
*
* Note that this value refers only to the two most significant bits of @c
* status_len. For details, see #ParamValue.
*
* Also, see #PARAM_STATUS_MASK, #PARAM_NOT_SET, #PARAM_INVALID, #PARAM_TOO_LONG.
*/
#define PARAM_VALID        (0xC0)
